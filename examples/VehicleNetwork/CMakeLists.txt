cmake_minimum_required(VERSION 3.16)

project(VehicleNetwork
        VERSION 1.0.0
        DESCRIPTION "AUTOSAR R22-11 – 5-Node Vehicle SOME/IP Network (Windows + Linux)"
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
# Build options
# ---------------------------------------------------------------------------
option(VEHICLE_BUILD_MCAL_TC34XX
    "Build the Aurix TC34xx MCAL stub library alongside the node executables"
    ON)

# ---------------------------------------------------------------------------
# Build the SOME/IP static library (adds 'someip' target)
# ---------------------------------------------------------------------------
add_subdirectory(${SOMEIP_ROOT} ${CMAKE_BINARY_DIR}/someip_lib EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------------------
# Shared include directories used by all node executables
# ---------------------------------------------------------------------------
set(VEHICLE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include          # VehicleServices.h, Compiler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platform         # Platform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/transport        # NodeTransport.h
    ${SOMEIP_ROOT}/include                       # SomeIp*.h, Std_Types.h, …
)

# ---------------------------------------------------------------------------
# Platform-specific compiler flags and libraries
#
# Windows:   Winsock2 (ws2_32) for UDP sockets
#            Kernel32 is linked automatically; no pthread needed
#            MSVC:  /W4 /wd4100 (unused param is normal in AUTOSAR stubs)
#            MinGW: same GCC flags as Linux
#
# Linux/macOS/QNX:  pthread for receive thread; standard POSIX flags
# ---------------------------------------------------------------------------
if(WIN32)
    set(PLATFORM_LIBS ws2_32)
    if(MSVC)
        set(VEHICLE_COMPILE_OPTS /W4 /wd4100 /wd4996)
    else()  # MinGW / MSYS2 GCC on Windows
        set(VEHICLE_COMPILE_OPTS
            -Wall -Wextra -Wcast-align -Wno-unused-parameter)
    endif()
else()  # POSIX
    set(PLATFORM_LIBS pthread)
    set(VEHICLE_COMPILE_OPTS
        -Wall
        -Wextra
        -Wcast-align
        -Wmissing-prototypes
        -Wstrict-prototypes
        -Wno-unused-parameter
    )
endif()

# ---------------------------------------------------------------------------
# NodeTransport static library (shared by all 5 nodes)
# On Windows: linked against ws2_32 for Winsock2 socket calls.
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
    PUBLIC  ${PLATFORM_LIBS}
)

# ---------------------------------------------------------------------------
# Optional: Aurix TC34xx MCAL stub library
# ---------------------------------------------------------------------------
if(VEHICLE_BUILD_MCAL_TC34XX)
    add_subdirectory(mcal/tc34xx)
endif()

# ---------------------------------------------------------------------------
# Macro: add_node_executable(name sources...)
# ---------------------------------------------------------------------------
function(add_node_executable NODE_NAME)
    add_executable(${NODE_NAME} ${ARGN})
    target_include_directories(${NODE_NAME} PRIVATE ${VEHICLE_INCLUDES})
    target_compile_features(${NODE_NAME} PRIVATE c_std_99)
    target_compile_options(${NODE_NAME}  PRIVATE ${VEHICLE_COMPILE_OPTS})
    target_link_libraries(${NODE_NAME}
        PRIVATE
            node_transport
            ${PLATFORM_LIBS}
    )
    # On Windows: embed a manifest so the console opens properly
    if(WIN32 AND MSVC)
        set_target_properties(${NODE_NAME} PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    endif()
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
# HPC node – ASAM SOVD 1.0 HTTP REST diagnostic gateway (SOME/IP consumer)
# Disable with -DVEHICLE_BUILD_HPC=OFF
# ---------------------------------------------------------------------------
option(VEHICLE_BUILD_HPC "Build the SOVD HPC diagnostic node" ON)
if(VEHICLE_BUILD_HPC)
    add_subdirectory(hpc)
endif()

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
set(INSTALL_TARGETS bcm_node ecm_node adas_node gateway_node ipc_node)
if(VEHICLE_BUILD_HPC)
    list(APPEND INSTALL_TARGETS hpc_node)
endif()
install(TARGETS ${INSTALL_TARGETS} RUNTIME DESTINATION bin)

if(WIN32)
    install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/launch_network.bat
            DESTINATION bin)
    install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/launch_network.ps1
            DESTINATION bin)
else()
    install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/launch_network.sh
            DESTINATION bin)
endif()

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
message(STATUS "----------------------------------------------------")
message(STATUS " VehicleNetwork – 5-node SOME/IP example")
message(STATUS " Platform    : ${CMAKE_SYSTEM_NAME}")
message(STATUS " Build type  : ${CMAKE_BUILD_TYPE}")
message(STATUS " SOMEIP_ROOT : ${SOMEIP_ROOT}")
message(STATUS " Compiler    : ${CMAKE_C_COMPILER}")
message(STATUS " Nodes       : bcm ecm adas gateway ipc hpc(sovd)")
if(VEHICLE_BUILD_MCAL_TC34XX)
message(STATUS " MCAL TC34xx : enabled (host-simulation mode)")
endif()
message(STATUS "----------------------------------------------------")
if(WIN32)
message(STATUS " To run (Windows CMD):  launch_network.bat <build_dir>")
message(STATUS " To run (PowerShell):   .\\launch_network.ps1 <build_dir>")
else()
message(STATUS " To run (Linux):        bash launch_network.sh <build_dir>")
endif()
message(STATUS "----------------------------------------------------")
