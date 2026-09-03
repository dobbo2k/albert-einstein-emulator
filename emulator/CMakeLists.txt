cmake_minimum_required(VERSION 3.24)
project(albert LANGUAGES C CXX)

# Link the C runtime statically. It costs a little size and removes three
# DLLs from anything shipped -- MSVCP140, VCRUNTIME140 and VCRUNTIME140_1 --
# so a release is the executable, SDL3.dll and nothing else to install. Set
# before any target (including the fetched SDL3 and ImGui) so every object
# agrees; mixing runtimes inside one binary does not link.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Do not rely on a default here -- something in this environment seeds the
# cache with Debug, and a Debug Z80 core makes zexall take many minutes.
# build.cmd passes -DCMAKE_BUILD_TYPE explicitly (RelWithDebInfo by default).
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()
message(STATUS "albert: build type = ${CMAKE_BUILD_TYPE}")

# Build the GUI frontend? OFF gives a dependency-free, network-free build of the
# core plus tests -- what CI and the M1 zexall harness want.
option(EIN_BUILD_APP "Build the SDL3/ImGui frontend" ON)

# The ROM archive lives one level up from this directory.
set(EIN_ROMS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../roms" CACHE PATH "Einstein ROM archive")
cmake_path(NORMAL_PATH EIN_ROMS_DIR)

add_subdirectory(src/core)

enable_testing()
add_subdirectory(tests)

if(EIN_BUILD_APP)
    include(cmake/deps.cmake)
    add_subdirectory(src/app)
endif()

if(EIN_BUILD_APP)
    include(cmake/dist.cmake)
endif()

message(STATUS "albert: app=${EIN_BUILD_APP} roms=${EIN_ROMS_DIR}")
