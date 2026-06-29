# hpc/CMakeLists.txt
# SOVD HPC node – HTTP REST diagnostic gateway for the vehicle SOME/IP network.
# Included from examples/VehicleNetwork/CMakeLists.txt via add_subdirectory().

# ── Sources ─────────────────────────────────────────────────────────────────
set(HPC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Sovd_DataStore.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Sovd_Server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main_hpc.c
)

add_executable(hpc_node ${HPC_SOURCES})

target_include_directories(hpc_node PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include  # Sovd_Types.h, Sovd_DataStore.h, Sovd_Server.h
    ${VEHICLE_INCLUDES}                  # VehicleServices.h, Platform.h, NodeTransport.h, SomeIp*.h
)

target_compile_features(hpc_node PRIVATE c_std_99)
target_compile_options(hpc_node  PRIVATE ${VEHICLE_COMPILE_OPTS})

target_link_libraries(hpc_node PRIVATE
    node_transport
    ${PLATFORM_LIBS}
)

# math library for sinf() (not needed on Windows/MSVC where it is part of CRT)
if(NOT WIN32)
    target_link_libraries(hpc_node PRIVATE m)
endif()

# ── Dashboard HTML ──────────────────────────────────────────────────────────
# Copy the dashboard HTML to the build directory so the server can find it at
# ./dashboard/index.html when launched from the build output folder.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/dashboard)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/dashboard/index.html
    ${CMAKE_BINARY_DIR}/dashboard/index.html
    COPYONLY
)

message(STATUS " HPC SOVD     : enabled (HTTP port 8080, UDP port 30506)")
