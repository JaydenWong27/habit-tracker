cmake_minimum_required(VERSION 3.16)
project(habit_tracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

include(FetchContent)

# nlohmann/json (you already had this)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# --- ftxui: TUI library ---
FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v5.0.0        # if this tag 404s for you, try v4.1.1
)
FetchContent_MakeAvailable(ftxui)

add_executable(habit_tracker
    src/main.cpp
    src/Habit.cpp
    src/HabitManager.cpp
)

target_link_libraries(habit_tracker
    PRIVATE
      nlohmann_json::nlohmann_json
      ftxui::component     # component, screen, dom
      ftxui::dom
      ftxui::screen
)
