add_unit_test(test_zone_powertrain
    test_torque_arbiter.cpp
    test_battery_manager.cpp
)
target_link_libraries(test_zone_powertrain PRIVATE sdv_powertrain_lib)
