cmake_minimum_required(VERSION 3.16)
project(someip_autosar C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

add_library(someip STATIC
    src/someip_serializer.c
    src/someip_sd.c
)

target_include_directories(someip PUBLIC include)

# SWE.5 / SWE.6 GTest suite – enabled by default, disable with -DSOMEIP_BUILD_TESTS=OFF
option(SOMEIP_BUILD_TESTS "Build SOME/IP SWE.5/SWE.6 GTest suite" ON)
if(SOMEIP_BUILD_TESTS)
    add_subdirectory(tests)
endif()
