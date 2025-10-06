#include "HabitManager.h"
#include <nlohmann/json.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace ftxui;

static const char* kDataFile = "habits.json";

// Helper: ISO string for (today - days_back)
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
    manager.load(kDataFile); // restore if exists

    // -------- UI State ----------
    int selected = 0;                 // which habit is highlighted
    bool adding = false;              // are we in "add habit" mode?
    std::string new_name;             // input buffer for new habit

    // Input + Button components for adding a habit:
    auto input = Input(&new_name, "new habit name");
    auto add_button = Button("Add", [&] {
        if (!new_name.empty()) {
            manager.addHabit(new_name);
            new_name.clear();
            adding = false;
        }
    });
    auto add_row = Container::Horizontal({input, add_button});

    // The main renderer: draws panels and handles basic layout
    auto renderer = Renderer(add_row, [&] {
        // Snapshot habit list (we'll query manager each frame)
        // For drawing today's checkmarks:
        std::string today = iso_days_back(0);

        // LEFT: list of habits with today-checkmarks
        Elements rows;
        // We'll need the size; if empty, keep selected at 0.
        // We will print each habit on one line with [✔]/[ ] and streak.
        // Also draw a selection indicator "▶".
        // To compute size, we use a small trick: we can't directly get vector<Habits>,
        // so we'll reuse list()'s info via a loop count. You can add an accessor
        // in HabitManager if you prefer. For now, we’ll just print using find() attempts.
        // Simpler: we will mirror names by saving them when we add. To keep code short,
        // we derive them from weeklyReport drawing below by asking HabitManager to save/load.
        // Instead, let's track names by reading from save file would be overkill.
        // -> Practical approach: temporarily expose habit names via a minimal helper.

        // We'll implement a tiny local lambda that extracts names by re-saving to JSON in memory:
        // (since Habit::toJson() is available)
        nlohmann::json j = nlohmann::json::array();
        manager.save(kDataFile); // ensure disk has up-to-date (optional)
        // A cleaner approach: extend HabitManager with getHabits(). For now, assume you added:
        //   const std::vector<Habit>& getHabits() const;
        // If you did, replace the JSON trick with direct access. I'll show both below.

        // ---------- Preferred: if you added getHabits() to HabitManager ----------
        Elements list_rows;
        {
            // BEGIN preferred block:
            // You need to add this method in HabitManager.h/.cpp:
            //   const std::vector<Habit>& getHabits() const { return habits_; }
            // Then use it here:
            extern const std::vector<Habit>& __get_habits_hack(const HabitManager&);
            // We'll define this friend-like helper at the end of this file temporarily,
            // then remove it once you add getHabits() properly.
            const auto& habits = __get_habits_hack(manager);

            if (habits.empty()) selected = 0;
            if (!habits.empty()) {
                if (selected >= (int)habits.size()) selected = (int)habits.size() - 1;
                if (selected < 0) selected = 0;
            }

            for (int i = 0; i < (int)habits.size(); ++i) {
                const auto& h = habits[i];
                bool done_today = h.isCompletedOn(today);
                auto sel_mark = text(i == selected ? "▶ " : "  ");
                auto box = text(done_today ? "[✔]" : "[ ]");
                auto name = text(" " + h.getName());
                auto streak = text("  (streak: " + std::to_string(h.currentStreak()) + ")");
                list_rows.push_back(hbox({sel_mark, box, name, streak}));
            }
        }

        // Assemble left panel element:
        auto left_panel = window(text(" Habits "),
                                 vbox(std::move(list_rows)) |
                                     size(WIDTH, GREATER_THAN, 30));

        // RIGHT: 7-day timeline for selected habit
        Element right_panel;
        {
            const auto& habits = __get_habits_hack(manager);
            if (!habits.empty()) {
                const auto& h = habits[selected];
                Elements days;
                for (int i = 6; i >= 0; --i) {
                    bool done = h.isCompletedOn(iso_days_back(i));
                    days.push_back(text(done ? "✔" : "✘") | center | size(WIDTH, EQUAL, 3));
                }
                auto title = text(" Weekly (last 7 days) for: " + h.getName());
                right_panel = window(title, hbox(std::move(days)) |
                                           size(HEIGHT, EQUAL, 3));
            } else {
                right_panel = window(text(" Weekly "), text("No habits"));
            }
        }

        // FOOTER: input row when adding, or instructions otherwise
        Element footer;
        if (adding) {
            footer = hbox({
                text("New habit: "),
                input->Render() | flex,
                text(" "),
                add_button->Render()
            });
        } else {
            footer = text("Keys: ↑/↓ move  | Space toggle today | a add  | s save | l load | q quit");
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

    // Handle key events: navigation, toggles, save/load, add mode, quit
    auto app = CatchEvent(renderer, [&](Event e) {
        const auto& habits = __get_habits_hack(manager);
        bool has = !habits.empty();
        // If in adding mode, pass events to input/button first.
        if (adding) {
            if (e == Event::Return) {
                if (!new_name.empty()) {
                    manager.addHabit(new_name);
                    new_name.clear();
                    adding = false;
                }
                return true;
            }
            // Let the input component process text events:
            if (input->OnEvent(e)) return true;
            if (add_button->OnEvent(e)) return true;
            if (e == Event::Escape) { adding = false; return true; }
            return false;
        }

        // Not in adding mode: handle global keys
        if (e == Event::Character('q') || e == Event::Escape) {
            manager.save(kDataFile);
            ScreenInteractive::Active()->ExitLoopClosure()();
            return true;
        }
        if (e == Event::Character('s')) {
            manager.save(kDataFile);
            return true;
        }
        if (e == Event::Character('l')) {
            manager.load(kDataFile);
            // Keep selection in range:
            if (has && selected >= (int)habits.size())
                selected = (int)habits.size() - 1;
            return true;
        }
        if (e == Event::Character('a')) {
            adding = true;
            return true;
        }
        if (e == Event::ArrowUp && has) {
            selected = std::max(0, selected - 1);
            return true;
        }
        if (e == Event::ArrowDown && has) {
            selected = std::min((int)habits.size() - 1, selected + 1);
            return true;
        }
        if (e == Event::Character(' ') && has) {
            // Toggle today's completion:
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

// ------- TEMP helper until you add getHabits() properly -------
// Add this method in HabitManager.h/.cpp:
//   const std::vector<Habit>& getHabits() const { return habits_; }
// Then remove the hack below and replace all calls to __get_habits_hack with getHabits().
const std::vector<Habit>& __get_habits_hack(const HabitManager& m) {
    // We need access to the vector for rendering.
    // Temporarily, declare this as a friend or move rendering into HabitManager.
    // For now, to keep you moving, change HabitManager to expose getHabits().
    extern const std::vector<Habit>& __expose_habits_from_manager(const HabitManager&);
    return __expose_habits_from_manager(m);
}
