add_library(sdv_body_lib STATIC
    src/LightingController.cpp
    src/DoorWindowController.cpp
    src/HvacController.cpp
    src/ComfortManager.cpp
    src/BodyServiceStub.cpp
)
target_include_directories(sdv_body_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(sdv_body_lib
    PUBLIC sdv_someip_middleware sdv_compiler_options
)

add_executable(sdv_body main.cpp)
target_link_libraries(sdv_body PRIVATE sdv_body_lib)
