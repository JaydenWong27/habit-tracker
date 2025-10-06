cmake_minimum_required(VERSION 3.16)
project(habit_tracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

include(FetchContent)

# --- JSON (header-only) ---
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# --- ftxui (TUI) ---
FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v4.1.1
)
FetchContent_MakeAvailable(ftxui)

# --- SQLite3 (system library via Homebrew) ---
find_package(SQLite3 REQUIRED)  # <-- needs `brew install sqlite`

add_executable(habit_tracker
  src/main.cpp
  src/Habit.cpp
  src/HabitManager.cpp
)

target_link_libraries(habit_tracker
  PRIVATE
    nlohmann_json::nlohmann_json
    ftxui::component
    ftxui::dom
    ftxui::screen
    SQLite::SQLite3
)
