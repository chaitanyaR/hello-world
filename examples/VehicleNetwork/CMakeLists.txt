cmake_minimum_required(VERSION 3.16)

project(VehicleNetwork
        VERSION 1.0.0
        DESCRIPTION "AUTOSAR R22-11 – 5-Node Vehicle SOME/IP Network Example"
        LANGUAGES C)

# ---------------------------------------------------------------------------
# SOMEIP_ROOT must be provided on the command line, e.g.:
#   cmake -S examples/VehicleNetwork -B build_vehicle \
#         -DSOMEIP_ROOT=${PWD}/someip
# ---------------------------------------------------------------------------
if(NOT DEFINED SOMEIP_ROOT)
    message(FATAL_ERROR
        "SOMEIP_ROOT is not set. "
        "Pass -DSOMEIP_ROOT=<path-to-someip> to cmake.")
endif()

# ---------------------------------------------------------------------------
# Build the SOME/IP static library (adds 'someip' target)
# ---------------------------------------------------------------------------
add_subdirectory(${SOMEIP_ROOT} ${CMAKE_BINARY_DIR}/someip_lib EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------------------
# Shared include directories used by all node executables
# ---------------------------------------------------------------------------
set(VEHICLE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include          # VehicleServices.h
    ${CMAKE_CURRENT_SOURCE_DIR}/transport        # NodeTransport.h
    ${SOMEIP_ROOT}/include                       # SomeIp*.h, Std_Types.h, …
)

# ---------------------------------------------------------------------------
# Shared compile options
# ---------------------------------------------------------------------------
set(VEHICLE_COMPILE_OPTS
    -Wall
    -Wextra
    -Wcast-align
    -Wmissing-prototypes
    -Wstrict-prototypes
    -Wno-unused-parameter          # SD hook stubs have unused params
)

# ---------------------------------------------------------------------------
# NodeTransport static library (shared by all 5 nodes)
# ---------------------------------------------------------------------------
add_library(node_transport STATIC
    transport/NodeTransport.c
)
target_include_directories(node_transport
    PUBLIC  ${VEHICLE_INCLUDES}
)
target_compile_features(node_transport PRIVATE c_std_99)
target_compile_options(node_transport  PRIVATE ${VEHICLE_COMPILE_OPTS})
target_link_libraries(node_transport
    PUBLIC  someip
    PRIVATE pthread
)

# ---------------------------------------------------------------------------
# Macro: add_node_executable(name sources...)
# Each node links: its own main + node_transport (which provides
# SomeIpSd_Transmit + the shared SOME/IP library).
# ---------------------------------------------------------------------------
function(add_node_executable NODE_NAME)
    add_executable(${NODE_NAME} ${ARGN})
    target_include_directories(${NODE_NAME} PRIVATE ${VEHICLE_INCLUDES})
    target_compile_features(${NODE_NAME} PRIVATE c_std_99)
    target_compile_options(${NODE_NAME}  PRIVATE ${VEHICLE_COMPILE_OPTS})
    target_link_libraries(${NODE_NAME}
        PRIVATE
            node_transport
            pthread
    )
endfunction()

# ---------------------------------------------------------------------------
# Five ECU node executables
# ---------------------------------------------------------------------------
add_node_executable(bcm_node     nodes/bcm_main.c)
add_node_executable(ecm_node     nodes/ecm_main.c)
add_node_executable(adas_node    nodes/adas_main.c)
add_node_executable(gateway_node nodes/gateway_main.c)
add_node_executable(ipc_node     nodes/ipc_main.c)

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
install(TARGETS bcm_node ecm_node adas_node gateway_node ipc_node
        RUNTIME DESTINATION bin)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/launch_network.sh
        DESTINATION bin)

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
message(STATUS "----------------------------------------------------")
message(STATUS " VehicleNetwork – 5-node SOME/IP example")
message(STATUS " Build type  : ${CMAKE_BUILD_TYPE}")
message(STATUS " SOMEIP_ROOT : ${SOMEIP_ROOT}")
message(STATUS " Compiler    : ${CMAKE_C_COMPILER}")
message(STATUS " Nodes       : bcm_node ecm_node adas_node gateway_node ipc_node")
message(STATUS "----------------------------------------------------")
message(STATUS " To run all nodes:  bash launch_network.sh <build_dir>")
message(STATUS "----------------------------------------------------")
