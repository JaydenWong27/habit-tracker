#pragma once
#include <string>
#include <nlohmann/json_fwd.hpp>

class Habit {
private:
    std::string name_;
    int streak_ = 0;
    bool completedToday_ = false;

public:
    explicit Habit(const std::string& habitName);

    void markComplete();
    void nextDay();                 // clears completedToday_
    int  getStreak() const;
    std::string getName() const;
    bool isCompletedToday() const;

    // JSON helpers
    nlohmann::json toJson() const;
    static Habit fromJson(const nlohmann::json& j);
};
