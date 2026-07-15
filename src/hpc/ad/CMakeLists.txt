add_library(sdv_hpc_ad STATIC
    src/AdStack.cpp
    src/Localizer.cpp
    src/MapManager.cpp
    src/TrajectoryGenerator.cpp
    src/BehaviorPlanner.cpp
    src/PredictionEngine.cpp
)
target_include_directories(sdv_hpc_ad
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(sdv_hpc_ad
    PUBLIC sdv_compiler_options
)
