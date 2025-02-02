cmake_minimum_required(VERSION 3.29)

project(factory_game)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-security")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-security")

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

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

set(SDL_CCACHE ON CACHE BOOL "" FORCE)
FetchContent_Declare(SDL3 URL https://github.com/libsdl-org/SDL/archive/refs/tags/preview-3.1.6.tar.gz)
FetchContent_MakeAvailable(SDL3)

set(IMGUI_SDL_TARGET sdl3 CACHE STRING "" FORCE)
FetchContent_Declare(IMGUI URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.6.tar.gz)
FetchContent_MakeAvailable(IMGUI)

add_library(imgui
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui.h
        ${imgui_SOURCE_DIR}/imconfig.h
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_internal.h
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imstb_rectpack.h
        ${imgui_SOURCE_DIR}/imstb_textedit.h
        ${imgui_SOURCE_DIR}/imstb_truetype.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp)

target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
target_link_libraries(imgui PUBLIC SDL3::SDL3)


# Check if generated file exists
set(GENERATED_RESOURCES_FILE "${CMAKE_SOURCE_DIR}/src/autogenerated/resources.h")
if(NOT EXISTS ${GENERATED_RESOURCES_FILE})
    message(FATAL_ERROR "Generated resources file not found. Please run the generator first!")
endif()

set(GENERATED_RECIPES_FILE "${CMAKE_SOURCE_DIR}/src/autogenerated/recipes.h")
if(NOT EXISTS ${GENERATED_RECIPES_FILE})
    message(FATAL_ERROR "Generated recipes file not found. Please run the generator first!")
endif()


add_executable(factory_game
    main.cpp
        src/core.h
        src/core.cpp
        src/sim.h
        src/sim.cpp
        src/storage.cpp
        src/storage.h
        src/factory.h
        src/autogenerated/resources.h
        src/autogenerated/recipes.h
        src/serialization.cpp
        src/serialization.h
        src/game/game.h
        src/tools/defer.h
        src/tools/gui.h
        src/tools/gui.cpp
        src/tools/format.h
        src/dsl/dsl.h
        src/dsl/examples.cpp
        src/dsl/examples.h
        src/game/navigation.h
        src/game/factory_overview.h
        src/game/factory_detail.h
)

add_executable(generators
    src/tools/generators.cpp
    src/tools/generators.h
)

add_executable(imgui_test
    imgui_test.cpp
        ${imgui_SOURCE_DIR}/imgui.h

)

add_subdirectory(tests)

target_link_libraries(factory_game PRIVATE nlohmann_json::nlohmann_json SDL3::SDL3 imgui)

target_link_libraries(generators PRIVATE nlohmann_json::nlohmann_json)

target_link_libraries(imgui_test PRIVATE imgui)