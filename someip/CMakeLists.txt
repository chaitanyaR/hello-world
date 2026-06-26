cmake_minimum_required(VERSION 3.16)
project(someip_autosar C CXX)

set(CMAKE_C_STANDARD   99)
set(CMAKE_CXX_STANDARD 17)

# Strict compilation – matches AUTOSAR code quality requirement SR-SOMEIP-102
add_compile_options(
    -Wall
    -Wextra
    -Wcast-align
    -Wmissing-prototypes
    -Wstrict-prototypes
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=strict-prototypes>
)

# --------------------------------------------------------------------------
# SomeIp static library
# AUTOSAR modules: SomeIp (Transformer) + SomeIpSd (Service Discovery)
# --------------------------------------------------------------------------
add_library(someip STATIC
    src/SomeIp.c
    src/SomeIp_SD.c
    src/Det.c
)

target_include_directories(someip PUBLIC include)

# --------------------------------------------------------------------------
# SWE.5 / SWE.6 GTest suite
# Enable with -DSOMEIP_BUILD_TESTS=ON (default ON)
# --------------------------------------------------------------------------
option(SOMEIP_BUILD_TESTS "Build SOME/IP SWE.5/SWE.6 GTest suite" ON)
if(SOMEIP_BUILD_TESTS)
    add_subdirectory(tests)
endif()
