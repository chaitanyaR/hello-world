add_library(sdv_powertrain_lib STATIC
    src/EngineController.cpp
    src/TorqueArbiter.cpp
    src/BatteryManager.cpp
    src/RegenBraking.cpp
    src/PowertrainServiceStub.cpp
)
target_include_directories(sdv_powertrain_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(sdv_powertrain_lib
    PUBLIC sdv_someip_middleware sdv_compiler_options
)

add_executable(sdv_powertrain main.cpp)
target_link_libraries(sdv_powertrain PRIVATE sdv_powertrain_lib)
