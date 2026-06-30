# In-process integration test: exercises all zone ECU components together
# without SOME/IP, verifying the full sensor → actuator data flow.
add_executable(integration_full_pipeline test_full_pipeline.cpp)
target_link_libraries(integration_full_pipeline
    PRIVATE
        sdv_adas_zone_lib
        sdv_powertrain_lib
        sdv_telemetry_lib
        sdv_body_lib
        GTest::gtest_main
)
gtest_discover_tests(integration_full_pipeline TEST_PREFIX "integration_")
