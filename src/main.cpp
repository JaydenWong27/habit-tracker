#include "HabitManager.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <algorithm>   // std::max, std::min
#include <chrono>
#include <ctime>
#include <string>

using namespace ftxui;

static const char* kDBFile = "habits.db";  // SQLite DB file

// Return ISO date for (today - days_back), e.g. "2025-10-05"
std::string iso_days_back(int days_back) {
    auto now = std::chrono::system_clock::now() - std::chrono::hours(24 * days_back);
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&tt);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return std::string(buf);
}

int main() {
    HabitManager manager;
    // open database and load habits
    manager.openDB(kDBFile);
    manager.loadFromDB(); // fills vector from DB

    // -------- UI State ----------
    int selected = 0;           // which habit row is highlighted
    bool adding = false;        // are we entering a new habit name?
    std::string new_name;       // input buffer for the new habit

    // Input & button used when `adding == true`
    auto input = Input(&new_name, "new habit name");
    auto add_button = Button("Add", [&] {
        if (!new_name.empty()) {
            manager.addHabit(new_name);
            new_name.clear();
            adding = false;
        }
    });
    auto add_row = Container::Horizontal({input, add_button});

    // Main renderer: draw left list, right 7-day view, and footer
    auto renderer = Renderer(add_row, [&] {
        const std::vector<Habit>& habits = manager.getHabits();
        std::string today = iso_days_back(0);

        // Keep selection in range
        if (habits.empty()) selected = 0;
        if (!habits.empty()) {
            if (selected >= (int)habits.size()) selected = (int)habits.size() - 1;
            if (selected < 0) selected = 0;
        }

        // LEFT: habit list with [✔]/[ ] today and streak
        Elements list_rows;
        for (int i = 0; i < (int)habits.size(); ++i) {
            const auto& h = habits[i];
            bool done_today = h.isCompletedOn(today);
            auto sel_mark = text(i == selected ? "▶ " : "  ");
            auto box = text(done_today ? "[✔]" : "[ ]");
            auto name = text(" " + h.getName());
            auto streak = text("  (streak: " + std::to_string(h.currentStreak()) + ")");
            list_rows.push_back(hbox({sel_mark, box, name, streak}));
        }
        auto left_panel = window(text(" Habits "),
                                 vbox(std::move(list_rows)) |
                                     size(WIDTH, GREATER_THAN, 30));

        // RIGHT: 7-day timeline for selected habit
        Element right_panel;
        if (!habits.empty()) {
            const auto& h = habits[selected];
            Elements days;
            for (int i = 6; i >= 0; --i) {
                bool done = h.isCompletedOn(iso_days_back(i));
                days.push_back(text(done ? "✔" : "✘") | center | size(WIDTH, EQUAL, 3));
            }
            auto title = text(" Weekly (last 7 days) for: " + h.getName());
            right_panel = window(title, hbox(std::move(days)) | size(HEIGHT, EQUAL, 3));
        } else {
            right_panel = window(text(" Weekly "), text("No habits"));
        }

        // FOOTER: input row (adding) or key help line
        Element footer;
        if (adding) {
            footer = hbox({
                text("New habit: "),
                input->Render() | flex,
                text(" "),
                add_button->Render()
            });
        } else {
            footer = text("Keys: ↑/↓ move  | Space toggle today | a add  | q quit");
        }

        // Layout: two columns + footer
        return vbox({
            hbox({
                left_panel | flex,
                separator(),
                right_panel | flex
            }) | flex,
            separator(),
            footer | center
        });
    });

    // Keyboard handling: arrows, toggle, add/quit
    auto app = CatchEvent(renderer, [&](Event e) {
        const auto& habits = manager.getHabits();
        bool has = !habits.empty();

        // When adding, route keys to the input/button first
        if (adding) {
            if (e == Event::Return) {
                if (!new_name.empty()) {
                    manager.addHabit(new_name);
                    new_name.clear();
                    adding = false;
                }
                return true;
            }
            if (input->OnEvent(e)) return true;
            if (add_button->OnEvent(e)) return true;
            if (e == Event::Escape) { adding = false; return true; }
            return false;
        }

        // Global keys
        if (e == Event::Character('q') || e == Event::Escape) {
            ScreenInteractive::Active()->ExitLoopClosure()();
            return true;
        }
        if (e == Event::Character('a')) { adding = true; return true; }

        if (e == Event::ArrowUp && has)  { selected = std::max(0, selected - 1); return true; }
        if (e == Event::ArrowDown && has){ selected = std::min((int)habits.size() - 1, selected + 1); return true; }

        if (e == Event::Character(' ') && has) {
            // Toggle today's completion for the selected habit
            bool done = habits[selected].isCompletedOn(iso_days_back(0));
            manager.setToday(habits[selected].getName(), !done);
            return true;
        }
        return false;
    });

    // Run the app full-screen
    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(app);
    return 0;
}
