cmake_minimum_required(VERSION 3.13)

project(r36s-flappy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CTest)

option(R36S_DRM "Use the raylib DRM/KMS backend for the R36S" OFF)
option(EMBED_ASSETS "Embed assets into the binary" OFF)

include(FetchContent)

if(R36S_DRM)
    add_library(raylib INTERFACE IMPORTED)
    target_include_directories(raylib INTERFACE /opt/raylib/src)
    target_link_libraries(raylib INTERFACE /tmp/raylib-build/raylib/libraylib.a drm gbm EGL GLESv2 pthread m dl)
else()
    set(PLATFORM SDL CACHE STRING "raylib platform backend" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(raylib)
endif()

if(R36S_DRM)
    set(RAYGUI_SRC_DIR "/opt/raygui/src")
else()
    FetchContent_Declare(raygui
        URL https://github.com/raysan5/raygui/archive/refs/tags/4.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(raygui)
    set(RAYGUI_SRC_DIR ${raygui_SOURCE_DIR}/src)
endif()

set(APP_SOURCES
    src/main.cpp
    src/app.cpp
    src/input_handler.cpp
    src/sprite_manager.cpp
    src/parallax_background.cpp
    src/screen_transition.cpp
    src/bgm_manager.cpp
    src/settings.cpp
    src/audio_gen.cpp
    src/game_logic.cpp
    src/game_renderer.cpp
    src/ui_renderer.cpp
    src/game/bird.cpp
    src/game/pipe_manager.cpp
    src/game/particle.cpp
    src/game/sound_manager.cpp
)

if(EMBED_ASSETS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(EMBEDDED_HEADER ${CMAKE_CURRENT_BINARY_DIR}/generated/embedded_assets.h)
    add_custom_command(
        OUTPUT ${EMBEDDED_HEADER}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated
        COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/scripts/embed_assets.py
            ${EMBEDDED_HEADER}
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/Montserrat-Medium.ttf montserrat_font
        DEPENDS assets/Montserrat-Medium.ttf scripts/embed_assets.py
        COMMENT "Embedding assets..."
    )
    list(APPEND APP_SOURCES ${EMBEDDED_HEADER})
    set_source_files_properties(${EMBEDDED_HEADER} PROPERTIES GENERATED TRUE)
endif()

add_executable(r36s-flappy ${APP_SOURCES})

target_include_directories(r36s-flappy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(r36s-flappy SYSTEM PRIVATE ${RAYGUI_SRC_DIR})
if(EMBED_ASSETS)
    target_include_directories(r36s-flappy PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
    target_compile_definitions(r36s-flappy PRIVATE EMBEDDED_ASSETS)
endif()
target_link_libraries(r36s-flappy PRIVATE raylib stdc++fs)

if(NOT EMBED_ASSETS)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

if(UNIX AND NOT APPLE)
    target_compile_options(r36s-flappy PRIVATE -Wall -Wextra -Wpedantic -O2)
endif()

if(BUILD_TESTING)
    add_executable(game_tests tests/game_tests.cpp src/game/bird.cpp src/game/pipe_manager.cpp src/game/particle.cpp)
    target_include_directories(game_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(game_tests PRIVATE raylib)
    add_test(NAME game_tests COMMAND game_tests)
endif()
