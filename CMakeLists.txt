cmake_minimum_required(VERSION 3.16)
project(habit_tracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(habit_tracker
    src/main.cpp
    src/Habit.cpp
    src/HabitManager.cpp
)

target_link_libraries(habit_tracker PRIVATE nlohmann_json::nlohmann_json)
