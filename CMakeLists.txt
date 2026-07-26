cmake_minimum_required(VERSION 3.21)

# The toolchain must be selected before project() so CMake can discover
# packages installed through vcpkg during its first configuration pass.
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Configure a native ARM64 toolchain before project() when vcpkg is not used.
option(BUILD_FOR_ARM64 "Cross-compile for ARM64" OFF)
if(BUILD_FOR_ARM64 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
endif()

project(r36s-app-cpp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find packages
find_package(SDL2 REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# LVGL is fetched from its upstream release because the vcpkg registry used by
# the R36S toolchain does not currently publish an lvgl port.
include(FetchContent)
# LVGL auxiliary targets (for example ThorVG) also include lv_conf.h.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
FetchContent_Declare(lvgl
    URL https://github.com/lvgl/lvgl/archive/refs/tags/v9.2.2.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(LV_CONF_BUILD_DISABLE_EXAMPLES ON CACHE BOOL "" FORCE)
set(LV_CONF_BUILD_DISABLE_DEMOS ON CACHE BOOL "" FORCE)
set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(lvgl)
target_compile_definitions(lvgl PUBLIC
    LV_CONF_INCLUDE_SIMPLE
    LV_DRAW_SW_SUPPORT_A8=1
)
target_include_directories(lvgl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Create executable
add_executable(r36s-hello
    src/main.cpp
    src/app.cpp
    src/input_manager.cpp
    src/weather/weather_client.cpp
    src/weather/weather_locations.cpp
    src/weather/weather_parser.cpp
    src/weather/settings_store.cpp
    src/weather/weather_screen.cpp
    src/weather/weather_screen_layout.cpp
    src/weather/weather_screen_view.cpp
    src/fonts/r36s_font_14.c
    src/fonts/r36s_font_16.c
    src/fonts/r36s_font_20.c
    src/fonts/r36s_font_48.c
    src/weather/icons/sun.c
    src/weather/icons/cloud-sun.c
    src/weather/icons/cloud.c
    src/weather/icons/cloud-rain.c
    src/weather/icons/cloud-snow.c
    src/weather/icons/cloud-fog.c
    src/weather/icons/cloud-lightning.c
    src/weather/icons/droplets.c
    src/weather/icons/wind.c
    src/weather/icons/chevrons-up-down.c
    src/weather/icons/chevrons-left-right.c
    src/weather/icons/circle.c
)

# Include directories
target_include_directories(r36s-hello PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(r36s-hello PRIVATE
    SDL2::SDL2
    lvgl
    CURL::libcurl
    nlohmann_json::nlohmann_json
    fmt::fmt
    spdlog::spdlog
)

# Compiler flags
if(UNIX AND NOT APPLE)
    target_compile_options(r36s-hello PRIVATE -Wall -Wextra -O2)
endif()

# Optional: Full-screen flag for R36S
option(R36S_FULLSCREEN "Build for fullscreen R36S display" OFF)
if(R36S_FULLSCREEN)
    target_compile_definitions(r36s-hello PRIVATE R36S_FULLSCREEN=1)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(weather_core_tests
        tests/weather_core_tests.cpp
        tests/icon_render_test.cpp
        src/weather/weather_client.cpp
        src/weather/weather_locations.cpp
        src/weather/weather_parser.cpp
        src/weather/settings_store.cpp
        src/weather/icons/cloud.c
    )
    target_include_directories(weather_core_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(weather_core_tests PRIVATE CURL::libcurl lvgl nlohmann_json::nlohmann_json fmt::fmt spdlog::spdlog)
    add_test(NAME weather_core_tests COMMAND weather_core_tests)
endif()
