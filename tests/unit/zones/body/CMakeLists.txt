add_unit_test(test_zone_body
    test_hvac_controller.cpp
    test_lighting_controller.cpp
)
target_link_libraries(test_zone_body PRIVATE sdv_body_lib)
