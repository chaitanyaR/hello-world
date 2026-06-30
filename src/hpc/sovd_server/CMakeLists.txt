add_library(sdv_hpc_sovd_server STATIC
    src/HpcSovdResource.cpp
    src/AdasSovdResource.cpp
    src/AdSovdResource.cpp
    src/ZoneForwarder.cpp
)
target_include_directories(sdv_hpc_sovd_server
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(sdv_hpc_sovd_server
    PUBLIC  sdv_sovd_middleware
            sdv_someip_middleware
            sdv_compiler_options
)
