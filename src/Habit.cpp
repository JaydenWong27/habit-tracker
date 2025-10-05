#include "Habit.h"
#include <nlohmann/json.hpp>
#include <ctime>
#include <iomanip>
#include <sstream>

using nlohmann::json;

Habit::Habit(const std::string& habitName) : name_(habitName) {}

void Habit::markCompleteToday() {
    completedDates_.insert(todayISO());
}

bool Habit::isCompletedOn(const std::string& date) const {
    return completedDates_.count(date) > 0;
}

int Habit::currentStreak() const {
    // Compute streak ending today
    // Start from today and go backwards while dates exist
    int streak = 0;
    // get today as time_t
    std::tm tm{};
    std::istringstream ss(todayISO());
    ss >> std::get_time(&tm, "%Y-%m-%d");
    std::time_t t = std::mktime(&tm);

    for (;;) {
        // format current t into string
        std::tm cur_tm = *std::localtime(&t);
        char buf[11];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d", &cur_tm);
        std::string curDate = buf;

        if (isCompletedOn(curDate)) {
            ++streak;
            // move to previous day
            t -= 24 * 60 * 60;
        } else {
            break;
        }
    }
    return streak;
}

std::string Habit::getName() const { return name_; }

json Habit::toJson() const {
    return json{
        {"name", name_},
        {"dates", completedDates_}
    };
}

Habit Habit::fromJson(const json& j) {
    Habit h(j.at("name").get<std::string>());
    h.completedDates_ = j.value("dates", std::set<std::string>{});
    return h;
}

std::string Habit::todayISO() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return std::string(buf);
}
