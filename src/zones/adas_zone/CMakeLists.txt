add_library(sdv_adas_zone_lib STATIC
    src/SensorAggregator.cpp
    src/RadarDriver.cpp
    src/LidarDriver.cpp
    src/CameraPreprocessor.cpp
    src/ActuatorController.cpp
    src/SafetyGateway.cpp
    src/AdasZoneServiceStub.cpp
)
target_include_directories(sdv_adas_zone_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(sdv_adas_zone_lib
    PUBLIC sdv_someip_middleware sdv_compiler_options
)

add_executable(sdv_adas_zone main.cpp)
target_link_libraries(sdv_adas_zone PRIVATE sdv_adas_zone_lib)
