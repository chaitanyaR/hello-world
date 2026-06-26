cmake_minimum_required(VERSION 3.16)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP true
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(someip_tests
    test_someip_serializer.cpp
    test_someip_sd.cpp
    stub_transport.c
)

target_link_libraries(someip_tests
    PRIVATE
        someip
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(someip_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(someip_tests)
