cmake_minimum_required(VERSION 3.16)

# ---------------------------------------------------------------------------
# mcal_tc34xx – Aurix TC34xx Ethernet MCAL stub library
#
# Build modes:
#   ETH_17_GETH_MAC_HOST_SIM (default ON):
#     Compiles to a host-runnable stub that delegates Transmit/Receive to
#     NodeTransport (UDP sockets).  Runs on Linux and Windows.
#
#   ETH_17_GETH_MAC_HOST_SIM OFF:
#     Compiles real register-level code; requires TriCore toolchain and
#     the TC34xx SDK (IfxEth headers from Infineon iLLD).
# ---------------------------------------------------------------------------

option(ETH_17_GETH_MAC_HOST_SIM
    "Build Eth_17_GEthMac in host-simulation mode (UDP sockets, no TC34xx HW)"
    ON)

# Sources
set(MCAL_SOURCES
    src/Eth_17_GEthMac.c
    src/EthIf.c
)

add_library(mcal_tc34xx STATIC ${MCAL_SOURCES})

# ---------------------------------------------------------------------------
# Include paths
# ---------------------------------------------------------------------------
target_include_directories(mcal_tc34xx
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include     # Mcal.h, Eth_17_GEthMac.h, …
    PRIVATE
        ${SOMEIP_ROOT}/include                  # SomeIp*.h, Std_Types.h
        ${CMAKE_SOURCE_DIR}/transport           # NodeTransport.h (host-sim)
        ${CMAKE_SOURCE_DIR}/include             # VehicleServices.h, Compiler.h
        ${CMAKE_SOURCE_DIR}/platform            # Platform.h
)

# ---------------------------------------------------------------------------
# Compile definitions and features
# ---------------------------------------------------------------------------
target_compile_features(mcal_tc34xx PRIVATE c_std_99)

if(ETH_17_GETH_MAC_HOST_SIM)
    target_compile_definitions(mcal_tc34xx PRIVATE ETH_17_GETH_MAC_HOST_SIM)
    message(STATUS " MCAL TC34xx : host-simulation mode (UDP loopback)")
else()
    message(STATUS " MCAL TC34xx : real hardware mode (requires TriCore toolchain)")
endif()

# ---------------------------------------------------------------------------
# Compiler warnings (GCC / Clang / MinGW; suppress MSVC-incompatible flags)
# ---------------------------------------------------------------------------
if(MSVC)
    target_compile_options(mcal_tc34xx PRIVATE /W4 /wd4100)
else()
    target_compile_options(mcal_tc34xx PRIVATE
        -Wall -Wextra -Wcast-align -Wno-unused-parameter)
endif()

# ---------------------------------------------------------------------------
# Link against node_transport for host-sim TX/RX
# ---------------------------------------------------------------------------
if(ETH_17_GETH_MAC_HOST_SIM)
    target_link_libraries(mcal_tc34xx PUBLIC node_transport)
endif()
