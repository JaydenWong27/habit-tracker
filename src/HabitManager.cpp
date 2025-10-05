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

bool HabitManager::markComplete(const std::string& name) {
    auto* h = find(name);
    if (!h) { std::cout << "(not found)\n"; return false; }
    h->markComplete();
    std::cout << "Marked complete: " << name << "\n";
    return true;
}

void HabitManager::list() const {
    if (habits_.empty()) { std::cout << "(no habits yet)\n"; return; }
    for (const auto& h : habits_) {
        std::cout << "- " << h.getName()
                  << " | streak: " << h.getStreak()
                  << (h.isCompletedToday() ? " | done today" : " | not done")
                  << "\n";
    }
}

void HabitManager::nextDay() {
    for (auto& h : habits_) h.nextDay();
    std::cout << "Advanced to next day. (resets today’s flags)\n";
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
