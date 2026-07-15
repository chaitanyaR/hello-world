add_library(sdv_hpc_adas STATIC
    src/AdasPipeline.cpp
    src/SensorFusion.cpp
    src/ObjectDetector.cpp
    src/ObjectTracker.cpp
    src/PathPlanner.cpp
    src/DecisionMaker.cpp
    src/SafetyMonitor.cpp
)
target_include_directories(sdv_hpc_adas
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(sdv_hpc_adas
    PUBLIC  sdv_compiler_options
            sdv_someip_middleware
)
