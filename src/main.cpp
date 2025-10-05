#include "HabitManager.h"
#include <iostream>
#include <sstream>

static const char* kDataFile = "habits.json";

void printMenu() {
    std::cout <<
      "\n=== Habit Tracker ===\n"
      "1) List habits\n"
      "2) Add habit\n"
      "3) Mark complete\n"
      "4) Next day\n"
      "5) Save\n"
      "6) Load\n"
      "7) Quit\n"
      "> ";
}

int main() {
    HabitManager manager;
    manager.load(kDataFile); // try to load on start

    while (true) {
        printMenu();
        std::string line; 
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        int choice = 0;
        try { choice = std::stoi(line); } catch (...) { choice = 0; }

        if (choice == 1) {
            manager.list();
        } else if (choice == 2) {
            std::cout << "New habit name: ";
            std::string name; std::getline(std::cin, name);
            if (name.empty()) { std::cout << "Name required.\n"; continue; }
            manager.addHabit(name);
        } else if (choice == 3) {
            std::cout << "Habit name to mark complete: ";
            std::string name; std::getline(std::cin, name);
            manager.markComplete(name);
        } else if (choice == 4) {
            manager.nextDay();
        } else if (choice == 5) {
            manager.save(kDataFile);
        } else if (choice == 6) {
            manager.load(kDataFile);
        } else if (choice == 7) {
            manager.save(kDataFile); // auto-save on exit (optional)
            std::cout << "Goodbye!\n";
            break;
        } else {
            std::cout << "Invalid choice.\n";
        }
    }
    return 0;
}
