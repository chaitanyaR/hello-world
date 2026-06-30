cmake_minimum_required(VERSION 3.22)
project(sdv_platform VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(cmake/CompilerOptions.cmake)

# --- External dependencies ---
# GTest/GMock built from source (available on Ubuntu via libgtest-dev)
add_subdirectory(/usr/src/googletest ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
find_package(nlohmann_json REQUIRED)

find_package(vsomeip3 QUIET)
find_package(CommonAPI QUIET)
find_package(CommonAPI-SomeIP QUIET)

option(SDV_SIMULATION_MODE "Build in simulation mode (no real hardware drivers)" ON)
option(ENABLE_COVERAGE "Enable gcov/lcov code coverage" OFF)

if(ENABLE_COVERAGE)
    include(cmake/CodeCoverage.cmake)
endif()

enable_testing()
include(GoogleTest)

# Alias GTest::gtest_main and GTest::gmock to the targets built from source
if(NOT TARGET GTest::gtest_main)
    add_library(GTest::gtest_main ALIAS gtest_main)
endif()
if(NOT TARGET GTest::gmock)
    add_library(GTest::gmock ALIAS gmock)
endif()
if(NOT TARGET GTest::gtest)
    add_library(GTest::gtest ALIAS gtest)
endif()

add_subdirectory(interfaces)
add_subdirectory(middleware)
add_subdirectory(src)
add_subdirectory(tests)
