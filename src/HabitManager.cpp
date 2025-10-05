#include "HabitManager.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using nlohmann::json;

void HabitManager::addHabit(const std::string& name) {
    if (find(name)) {
        std::cout << "Habit already exists.\n";
        return;
    }
    habits_.emplace_back(name);
    std::cout << "Added: " << name << "\n";
}

void HabitManager::list() const {
    if (habits_.empty()) { std::cout << "(no habits yet)\n"; return; }
    for (const auto& h : habits_) {
        std::cout << "- " << h.getName()
          << " | streak: " << h.currentStreak()
          << (h.isCompletedOn(Habit::todayISO()) ? " | done today" : " | not done")
          << "\n";
    }
}

Habit* HabitManager::find(const std::string& name) {
    for (auto& h : habits_) if (h.getName() == name) return &h;
    return nullptr;
}
const Habit* HabitManager::find(const std::string& name) const {
    for (auto& h : habits_) if (h.getName() == name) return &h;
    return nullptr;
}

// ---------- persistence ----------
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
    for (const auto& item : j) {
        habits_.push_back(Habit::fromJson(item));
    }
    std::cout << "Loaded " << habits_.size() << " habit(s) from " << path << "\n";
    return true;
}

bool HabitManager::markCompleteToday(const std::string& name) {
    auto* h = find(name);
    if (!h) { std::cout << "(not found)\n"; return false; }
    h->markCompleteToday();
    std::cout << "Marked today complete: " << name << "\n";
    return true;
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

