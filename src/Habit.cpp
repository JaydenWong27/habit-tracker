#include "Habit.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sqlite3.h>

using nlohmann::json;

Habit::Habit(const std::string& habitName) : name_(habitName) {}

void Habit::markCompleteToday() { completedDates_.insert(todayISO()); }
void Habit::unmarkToday()       { completedDates_.erase(todayISO()); }

bool Habit::isCompletedOn(const std::string& date) const {
    return completedDates_.count(date) > 0;
}

int Habit::currentStreak() const {
    // Walk backward from today; stop at the first missing day.
    int streak = 0;
    auto day = std::chrono::system_clock::now();
    while (true) {
        std::time_t tt = std::chrono::system_clock::to_time_t(day);
        std::tm tm = *std::localtime(&tt);
        char buf[11];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
        if (isCompletedOn(buf)) {
            ++streak;
            day -= std::chrono::hours(24);
        } else break;
    }
    return streak;
}

std::string Habit::getName() const { return name_; }

json Habit::toJson() const {
    return json{{"name", name_}, {"dates", completedDates_}};
}

Habit Habit::fromJson(const json& j) {
    Habit h(j.at("name").get<std::string>());
    h.completedDates_ = j.value("dates", std::set<std::string>{});
    return h;
}

std::string Habit::todayISO() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&tt);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return std::string(buf);
}
