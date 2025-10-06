#pragma once
#include <string>
#include <set>
#include <nlohmann/json_fwd.hpp>

class Habit {
private:
    std::string name_;
    std::set<std::string> completedDates_;   // "YYYY-MM-DD"

public:
    explicit Habit(const std::string& habitName);

    void markCompleteToday();
    void unmarkToday();                       // toggle off today
    bool isCompletedOn(const std::string& date) const;

    int  currentStreak() const;               // compute from completedDates_
    std::string getName() const;

    nlohmann::json toJson() const;
    static Habit fromJson(const nlohmann::json& j);

    static std::string todayISO();            // return today's date as "YYYY-MM-DD"
};
