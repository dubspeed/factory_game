cmake_minimum_required(VERSION 3.29)

project(factory_game)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(factory_game
    main.cpp
        src/core.h
        src/core.cpp
        src/sim.h
        src/sim.cpp
        src/storage.cpp
        src/storage.h
        src/factory.h
)

add_subdirectory(tests)

target_link_libraries(factory_game PRIVATE nlohmann_json::nlohmann_json)
