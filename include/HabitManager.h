#pragma once
#include "Habit.h"
#include <vector>
#include <string>

class HabitManager {
public:
    // CRUD
    void addHabit(const std::string& name);

    // Mark "today" complete for a specific habit (replaces markComplete)
    bool markCompleteToday(const std::string& name);

    // Display all habits with basic info
    void list() const;

    // OPTIONAL now that we track dates; you can delete this if unused.
    // If you keep it, it can be a no-op or used to simulate advancing the day.
    void nextDay();

    // Reports / analytics
    void weeklyReport() const;  // NEW: show last 7 days with ✔/✘ per habit

    // persistence
    bool save(const std::string& path) const;
    bool load(const std::string& path);

    bool setToday(const std::string& name, bool done); // NEW

    const std::vector<Habit>* getHabits() const;


private:
    std::vector<Habit> habits_;

    // Lookup helpers
    Habit* find(const std::string& name);
    const Habit* find(const std::string& name) const;
};
