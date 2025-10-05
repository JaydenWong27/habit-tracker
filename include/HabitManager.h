#pragma once
#include "Habit.h"
#include <vector>
#include <string>

class HabitManager {
public:
    // CRUD
    void addHabit(const std::string& name);
    bool markComplete(const std::string& name);
    void list() const;
    void nextDay();

    // persistence
    bool save(const std::string& path) const;
    bool load(const std::string& path);

private:
    std::vector<Habit> habits_;
    Habit* find(const std::string& name);
    const Habit* find(const std::string& name) const;
};
