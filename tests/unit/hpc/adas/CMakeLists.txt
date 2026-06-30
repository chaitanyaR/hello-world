add_unit_test(test_hpc_adas
    test_sensor_fusion.cpp
    test_object_tracker.cpp
    test_decision_maker.cpp
)
target_link_libraries(test_hpc_adas PRIVATE sdv_hpc_adas sdv_test_mocks)
