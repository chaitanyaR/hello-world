add_subdirectory(unit)
add_subdirectory(mocks)
# integration tests require simulation mode
if(SDV_SIMULATION_MODE)
    add_subdirectory(integration)
endif()
