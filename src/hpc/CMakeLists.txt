add_subdirectory(adas)
add_subdirectory(ad)
add_subdirectory(sovd_server)

add_executable(sdv_hpc main.cpp)
target_link_libraries(sdv_hpc
    PRIVATE sdv_hpc_adas
            sdv_hpc_ad
            sdv_hpc_sovd_server
            sdv_sovd_middleware
            sdv_someip_middleware
            sdv_compiler_options
)
