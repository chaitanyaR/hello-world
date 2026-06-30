add_unit_test(test_sovd_router
    test_sovd_router.cpp
)
target_link_libraries(test_sovd_router PRIVATE sdv_sovd_middleware)

add_unit_test(test_sovd_fault_memory
    test_sovd_fault_memory.cpp
)
target_link_libraries(test_sovd_fault_memory PRIVATE sdv_sovd_middleware)

add_unit_test(test_sovd_authentication
    test_sovd_authentication.cpp
)
target_link_libraries(test_sovd_authentication PRIVATE sdv_sovd_middleware)
