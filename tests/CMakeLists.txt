cmake_minimum_required(VERSION 3.29)

project(tests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_program(CCACHE_EXECUTABLE ccache)
mark_as_advanced(CCACHE_EXECUTABLE)
if(CCACHE_EXECUTABLE)
    foreach(LANG C CXX)
        if(NOT DEFINED CMAKE_${LANG}_COMPILER_LAUNCHER AND NOT CMAKE_${LANG}_COMPILER MATCHES ".*/ccache")
            message(STATUS "Enabling ccache for ${LANG}")
            set(CMAKE_${LANG}_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE} CACHE STRING "")
        endif()
    endforeach()
endif()

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
        machine_tests.cpp
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
        factory_tests.cpp
        ../src/serialization.cpp
        reconnect_links_tests.cpp
        ../src/serialization.h
        extractor_tests.cpp
        removal_tests.cpp
        ../src/game/game.h
)

target_compile_definitions(factory_tests PRIVATE TESTING)

target_link_libraries(
        factory_tests
        PRIVATE nlohmann_json::nlohmann_json GTest::gtest_main
)



include(GoogleTest)
gtest_discover_tests(factory_tests)
