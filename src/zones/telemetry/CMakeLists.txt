add_library(sdv_telemetry_lib STATIC
    src/TelemetryPublisher.cpp
    src/OtaManager.cpp
    src/HmiAdapter.cpp
    src/V2xGateway.cpp
    src/ConnectivityManager.cpp
    src/TelemetryServiceStub.cpp
)
target_include_directories(sdv_telemetry_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(sdv_telemetry_lib
    PUBLIC sdv_someip_middleware sdv_compiler_options
)

add_executable(sdv_telemetry main.cpp)
target_link_libraries(sdv_telemetry PRIVATE sdv_telemetry_lib)
