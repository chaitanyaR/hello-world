add_library(sdv_sovd_middleware STATIC
    src/SovdServer.cpp
    src/SovdRouter.cpp
    src/SovdResource.cpp
    src/SovdFaultMemory.cpp
    src/SovdRoutine.cpp
    src/SovdDataAccessor.cpp
    src/SovdAuthentication.cpp
)

target_include_directories(sdv_sovd_middleware
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(sdv_sovd_middleware
    PUBLIC  sdv_compiler_options
            nlohmann_json::nlohmann_json
    # Pistache is optional during Phase 1; link when available
    $<$<TARGET_EXISTS:pistache>:pistache>
)
