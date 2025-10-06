#pragma once
#include "Habit.h"
#include <vector>
#include <string>
#include <sqlite3.h>

class HabitManager {
public:
    HabitManager();                       // constructor
    ~HabitManager();                      // destructor

    // open a database file (call this at startup)
    bool openDB(const std::string& path);

    void addHabit(const std::string& name);
    bool markCompleteToday(const std::string& name);
    void list() const;
    void weeklyReport() const;
    bool loadFromDB(); 

    // keep JSON save/load if you want to export
    bool save(const std::string& path) const;
    bool load(const std::string& path);

    // toggle today's completion
    bool setToday(const std::string& name, bool done);

    // expose habits to the UI
    const std::vector<Habit>& getHabits() const;

private:
    std::vector<Habit> habits_;

    // NEW: SQLite database handle
    sqlite3* db_ = nullptr;

    Habit* find(const std::string& name);
    const Habit* find(const std::string& name) const;

    // helper to get habit id from DB (will implement in .cpp)
    int getHabitId(const std::string& name);
};
