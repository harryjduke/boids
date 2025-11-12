# --- SETUP ---
cmake_minimum_required(VERSION 3.21) # required to set CMAKE_TOOLCHAIN_FILE

# Setup vcpkg
if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if (DEFINED ENV{VCPKG_ROOT})
        cmake_path(SET CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        message(STATUS "Using toolchain file from VCPKG_ROOT: ${CMAKE_TOOLCHAIN_FILE}")
    endif ()
else ()
    message(STATUS "Using provided CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
endif ()

# Setup project
project(boids C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Setup build output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# --- DEPENDENCIES ---
if (EXISTS "${CMAKE_SOURCE_DIR}/lib")
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/lib")
    message(STATUS "Local /lib folder will be checked for dependencies.")
endif ()

find_package(raylib REQUIRED)
find_path(RAYGUI_INCLUDE_DIRS "raygui.h")

# --- BUILD PROJECT ---

add_executable(game
    src/main.c
    src/boid.c
)

target_include_directories(game PRIVATE ${RAYGUI_INCLUDE_DIRS})
target_include_directories(game PRIVATE include)
target_link_libraries(game PRIVATE raylib)

# Set startup project (for visual studio solution generation)
if (CMAKE_GENERATOR MATCHES "Visual Studio")
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif ()
