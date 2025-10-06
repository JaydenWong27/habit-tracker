#include "HabitManager.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

using nlohmann::json;

// ----------------- Constructor / Destructor -----------------

HabitManager::HabitManager() = default;

HabitManager::~HabitManager() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

// open or create the database and ensure tables exist
bool HabitManager::openDB(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "Cannot open DB: " << sqlite3_errmsg(db_) << "\n";
        db_ = nullptr;
        return false;
    }
    const char* habits_sql =
        "CREATE TABLE IF NOT EXISTS habits("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT UNIQUE);";
    sqlite3_exec(db_, habits_sql, nullptr, nullptr, nullptr);

    const char* completions_sql =
        "CREATE TABLE IF NOT EXISTS completions("
        "habit_id INTEGER,"
        "date TEXT,"
        "PRIMARY KEY(habit_id,date));";
    sqlite3_exec(db_, completions_sql, nullptr, nullptr, nullptr);

    return true;
}

// ----------------- Add / Find -----------------

void HabitManager::addHabit(const std::string& name) {
    if (find(name)) {
        std::cout << "Habit already exists.\n";
        return;
    }
    habits_.emplace_back(name);

    // also insert into DB if available
    if (db_) {
        sqlite3_stmt* stmt;
        const char* sql = "INSERT OR IGNORE INTO habits(name) VALUES(?);";
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    std::cout << "Added: " << name << "\n";
}

const std::vector<Habit>& HabitManager::getHabits() const {
    return habits_;
}

Habit* HabitManager::find(const std::string& name) {
    for (auto& h : habits_)
        if (h.getName() == name) return &h;
    return nullptr;
}
const Habit* HabitManager::find(const std::string& name) const {
    for (const auto& h : habits_)
        if (h.getName() == name) return &h;
    return nullptr;
}

// helper to get habit id from DB
int HabitManager::getHabitId(const std::string& name) {
    if (!db_) return -1;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id FROM habits WHERE name=?;";
    int id = -1;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return id;
}

// ----------------- Listing & Reporting -----------------

void HabitManager::list() const {
    if (habits_.empty()) { std::cout << "(no habits yet)\n"; return; }
    for (const auto& h : habits_) {
        std::cout << "- " << h.getName()
                  << " | streak: " << h.currentStreak()
                  << (h.isCompletedOn(Habit::todayISO()) ? " | done today" : " | not done")
                  << "\n";
    }
}

void HabitManager::weeklyReport() const {
    if (habits_.empty()) { std::cout << "(no habits)\n"; return; }

    std::cout << "\n=== Weekly Report (last 7 days) ===\n";
    auto now = std::chrono::system_clock::now();

    for (const auto& h : habits_) {
        std::cout << h.getName() << " | streak: " << h.currentStreak() << "\n  ";

        for (int i = 6; i >= 0; --i) {
            auto t  = now - std::chrono::hours(24 * i);
            std::time_t tt = std::chrono::system_clock::to_time_t(t);
            std::tm tm = *std::localtime(&tt);

            char buf[11];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
            std::string date = buf;

            std::cout << (h.isCompletedOn(date) ? "✔ " : "✘ ");
        }
        std::cout << "\n";
    }
}

// ----------------- Marking -----------------

bool HabitManager::markCompleteToday(const std::string& name) {
    auto* h = find(name);
    if (!h) { std::cout << "(not found)\n"; return false; }
    h->markCompleteToday();

    // also insert completion into DB
    if (db_) {
        int habit_id = getHabitId(name);
        if (habit_id != -1) {
            std::string today = Habit::todayISO();
            sqlite3_stmt* stmt;
            const char* sql = "INSERT OR REPLACE INTO completions(habit_id,date) VALUES(?,?);";
            if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, habit_id);
                sqlite3_bind_text(stmt, 2, today.c_str(), -1, SQLITE_STATIC);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
    }

    std::cout << "Marked today complete: " << name << "\n";
    return true;
}

bool HabitManager::setToday(const std::string& name, bool done) {
    auto* h = find(name);
    if (!h) { std::cout << "(not found)\n"; return false; }
    if (done) h->markCompleteToday();
    else      h->unmarkToday();

    // also update DB completions
    if (db_) {
        int habit_id = getHabitId(name);
        std::string today = Habit::todayISO();
        if (done) {
            sqlite3_exec(db_, ("INSERT OR REPLACE INTO completions(habit_id,date) "
                               "VALUES(" + std::to_string(habit_id) + ",'" + today + "');").c_str(),
                         nullptr, nullptr, nullptr);
        } else {
            sqlite3_exec(db_, ("DELETE FROM completions WHERE habit_id=" +
                               std::to_string(habit_id) + " AND date='" + today + "';").c_str(),
                         nullptr, nullptr, nullptr);
        }
    }

    return true;
}

// ----------------- Persistence (JSON export still works) -----------------

bool HabitManager::save(const std::string& path) const {
    json j = json::array();
    for (const auto& h : habits_) j.push_back(h.toJson());
    std::ofstream out(path);
    if (!out) { std::cout << "Could not open " << path << " for write.\n"; return false; }
    out << j.dump(2);
    std::cout << "Saved to " << path << "\n";
    return true;
}

bool HabitManager::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) { std::cout << "No existing data at " << path << " (starting fresh)\n"; return false; }
    json j; in >> j;
    habits_.clear();
    for (const auto& item : j) habits_.push_back(Habit::fromJson(item));
    std::cout << "Loaded " << habits_.size() << " habit(s) from " << path << "\n";
    return true;
}

// ----------------- Accessor for UI -----------------
bool HabitManager::loadFromDB() {
    if (!db_) return false;
    habits_.clear();

    const char* sql = "SELECT name FROM habits;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            habits_.emplace_back(name);
            // later you can also read completions here if needed
        }
        sqlite3_finalize(stmt);
    }
    std::cout << "Loaded " << habits_.size() << " habit(s) from DB.\n";
    return true;
}

