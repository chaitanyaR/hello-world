add_library(sdv_someip_middleware STATIC
    src/ServiceRegistry.cpp
    src/ServiceProxy.cpp
    src/ServiceStub.cpp
    src/SomeIpConfig.cpp
)

target_include_directories(sdv_someip_middleware
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(sdv_someip_middleware
    PUBLIC  sdv_compiler_options
    $<$<BOOL:${vsomeip3_FOUND}>:vsomeip3>
)

target_compile_definitions(sdv_someip_middleware
    PUBLIC $<$<BOOL:${SDV_SIMULATION_MODE}>:SDV_SIMULATION_MODE>
)

# Install vsomeip JSON configs alongside binaries so VSOMEIP_CONFIGURATION
# can be set per-process in the simulation launch script.
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config/
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config/someip)
