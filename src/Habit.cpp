#include "Habit.h"
#include <nlohmann/json.hpp>

using nlohmann::json;

Habit::Habit(const std::string& habitName) : name_(habitName) {}

void Habit::markComplete() {
    if (!completedToday_) {
        ++streak_;
        completedToday_ = true;
    }
}

void Habit::nextDay() {
    completedToday_ = false; // simple model: keep streak; reset today’s flag
}

int Habit::getStreak() const { return streak_; }
std::string Habit::getName() const { return name_; }
bool Habit::isCompletedToday() const { return completedToday_; }

// -------- JSON --------
json Habit::toJson() const {
    return json{
        {"name", name_},
        {"streak", streak_},
        {"completedToday", completedToday_}
    };
}

Habit Habit::fromJson(const json& j) {
    Habit h(j.at("name").get<std::string>());
    h.streak_ = j.value("streak", 0);
    h.completedToday_ = j.value("completedToday", false);
    return h;
}
