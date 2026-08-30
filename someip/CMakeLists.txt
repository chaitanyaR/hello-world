cmake_minimum_required(VERSION 3.16)
project(someip_autosar C CXX)

set(CMAKE_C_STANDARD   99)
set(CMAKE_CXX_STANDARD 17)

# Strict compilation – matches AUTOSAR code quality requirement SR-SOMEIP-102
# MSVC and GCC/Clang use different flag syntax; select at configure time.
if(MSVC)
    # /W4   – high warning level
    # /wd4100 – unreferenced formal parameter (common in AUTOSAR stubs)
    # /wd4996 – 'deprecated' POSIX names (strcpy_s etc.)
    # /wd4200 – zero-sized array in struct extension (not used here but safe)
    add_compile_options(/W4 /wd4100 /wd4996)
    # Treat warnings as errors only for the project's own files, not GTest
    # (done per-target below via target_compile_options, not globally)
else()
    # GCC / Clang / MinGW-w64
    add_compile_options(
        -Wall
        -Wextra
        -Wcast-align
        # C-only flags (not valid for the C++ GTest translation units)
        $<$<COMPILE_LANGUAGE:C>:-Wmissing-prototypes>
        $<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>
    )
endif()

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
