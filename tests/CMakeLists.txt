cmake_minimum_required(VERSION 3.29)

project(tests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        factory_tests

        ../src/core.h
        ../src/core.cpp
        recipe_tests.cpp
        integration_tests.cpp
        single_machine_tests.cpp
        one_one_connection_tests.cpp
        belt_tests.cpp
        stack_tests.cpp
        splitter_tests.cpp
        merger_tests.cpp
        ../src/sim.h
        ../src/sim.cpp
        ../src/storage.cpp
        ../src/storage.h
        storage_tests.cpp
        ../src/factory.h
        json_tests.cpp
        gameworld_tests.cpp
        ../src/serialization.cpp
        reconnect_links_tests.cpp
        ../src/serialization.h
        extractor_tests.cpp
)

target_compile_definitions(factory_tests PRIVATE TESTING)

target_link_libraries(
        factory_tests
        PRIVATE nlohmann_json::nlohmann_json GTest::gtest_main
)



include(GoogleTest)
gtest_discover_tests(factory_tests)
