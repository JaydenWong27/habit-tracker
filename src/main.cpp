#include "HabitManager.h"
#include <iostream>
#include <limits>   // for std::numeric_limits

static const char* kDataFile = "habits.json";

// Print the menu once per loop
void printMenu() {
    std::cout <<
      "\n=== Habit Tracker ===\n"
      "1) List habits\n"
      "2) Add habit\n"
      "3) Mark today complete\n"
      "4) Weekly report (last 7 days)\n"
      "5) Save\n"
      "6) Load\n"
      "7) Quit\n"
      "> ";
}

int main() {
    HabitManager manager;

    // Try to restore previous data; if file doesn't exist, we just start fresh
    manager.load(kDataFile);

    while (true) {
        printMenu();

        // Read a numeric choice robustly (clear bad input)
        int choice = 0;
        if (!(std::cin >> choice)) {
            std::cin.clear(); // clear error flags
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard line
            std::cout << "Please enter a number 1-7.\n";
            continue;
        }
        // consume the leftover newline so getline() works next
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 1) {
            manager.list();

        } else if (choice == 2) {
            std::cout << "New habit name: ";
            std::string name;
            std::getline(std::cin, name);
            if (name.empty()) {
                std::cout << "Name required.\n";
            } else {
                manager.addHabit(name);
            }

        } else if (choice == 3) {
            std::cout << "Habit name: ";
            std::string name;
            std::getline(std::cin, name);
            if (!name.empty()) manager.markCompleteToday(name);

        } else if (choice == 4) {
            manager.weeklyReport();

        } else if (choice == 5) {
            manager.save(kDataFile);

        } else if (choice == 6) {
            manager.load(kDataFile);

        } else if (choice == 7) {
            // Optional auto-save on exit
            manager.save(kDataFile);
            std::cout << "Goodbye!\n";
            break;

        } else {
            std::cout << "Invalid choice. Please pick 1-7.\n";
        }
    }

    return 0;
}
