function(add_unit_test target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name}
        PRIVATE GTest::gtest_main GTest::gmock sdv_test_mocks
    )
    gtest_discover_tests(${target_name} TEST_PREFIX "unit_")
endfunction()

add_subdirectory(middleware)
add_subdirectory(hpc)
add_subdirectory(zones)
