add_unit_test(test_hpc_ad
    test_localizer.cpp
    test_behavior_planner.cpp
)
target_link_libraries(test_hpc_ad PRIVATE sdv_hpc_ad)
