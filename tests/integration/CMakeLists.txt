# Integration tests require simulation mode (all 5 processes on loopback).
# Implemented in Phase 2/3.

# add_executable(integration_hpc_powertrain_someip test_hpc_powertrain_someip.cpp)
# target_link_libraries(integration_hpc_powertrain_someip
#     PRIVATE sdv_someip_middleware GTest::gtest_main)
# gtest_discover_tests(integration_hpc_powertrain_someip TEST_PREFIX "integration_")

# add_executable(integration_sovd_rest_api test_sovd_rest_api.cpp)
# target_link_libraries(integration_sovd_rest_api
#     PRIVATE sdv_sovd_middleware GTest::gtest_main)
# gtest_discover_tests(integration_sovd_rest_api TEST_PREFIX "integration_")
