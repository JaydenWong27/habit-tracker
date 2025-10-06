#pragma once
#include <string>
#include <set>
#include <chrono>
#include <nlohmann/json_fwd.hpp>

class Habit {
private:
    std::string name_;
    std::set<std::string> completedDates_; // store ISO date strings (YYYY-MM-DD)

public:
    explicit Habit(const std::string& habitName);

    void markCompleteToday();
    bool isCompletedOn(const std::string& date) const;

    int  currentStreak() const;            // compute streak ending today
    std::string getName() const;

    nlohmann::json toJson() const;
    static Habit fromJson(const nlohmann::json& j);

    // Helper for dates
    static std::string todayISO();
public:
    explicit Habit(const std::string& habitName);

    void markCompleteToday();
    void unmarkToday();                 // NEW: allow toggling off today
    bool isCompletedOn(const std::string& date) const;

    int  currentStreak() const;
    std::string getName() const;

    nlohmann::json toJson() const;
    static Habit fromJson(const nlohmann::json& j);

    static std::string todayISO();
};
