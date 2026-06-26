cmake_minimum_required(VERSION 3.16)
project(someip_autosar C)

set(CMAKE_C_STANDARD 99)

add_library(someip STATIC
    src/someip_serializer.c
    src/someip_sd.c
)

target_include_directories(someip PUBLIC include)

# Optional: build a simple integration test
option(SOMEIP_BUILD_TESTS "Build SOME/IP unit tests" OFF)
if(SOMEIP_BUILD_TESTS)
    add_subdirectory(tests)
endif()
