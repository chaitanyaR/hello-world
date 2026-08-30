cmake_minimum_required(VERSION 3.16)

project(BulkDataTransfer
        VERSION 1.0.0
        DESCRIPTION "AUTOSAR R22-11 BulkDataTransfer Example Application"
        LANGUAGES C)

# ---------------------------------------------------------------------------
# SOMEIP_ROOT must be provided on the command line, e.g.:
#   cmake -S examples/BulkDataTransfer -B build_example \
#         -DSOMEIP_ROOT=${PWD}/someip
# ---------------------------------------------------------------------------
if(NOT DEFINED SOMEIP_ROOT)
    message(FATAL_ERROR
        "SOMEIP_ROOT is not set. "
        "Pass -DSOMEIP_ROOT=<path-to-someip> to cmake.")
endif()

# ---------------------------------------------------------------------------
# Build the SOME/IP static library from SOMEIP_ROOT (if not already built)
# ---------------------------------------------------------------------------
add_subdirectory(${SOMEIP_ROOT} ${CMAKE_BINARY_DIR}/someip_lib EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------
set(APP_SOURCES
    src/main.c
    src/Os.c
    src/Os_Cfg.c
    src/Rte_BulkDataTransfer.c
    src/BulkDataTransfer.c
    src/SomeIpSd_Transport.c    # BSW transport stub (replaces SoAd on host)
)

# ---------------------------------------------------------------------------
# Executable target
# ---------------------------------------------------------------------------
add_executable(BulkDataTransfer ${APP_SOURCES})

# ---------------------------------------------------------------------------
# Include directories
#   1. Application headers (Compiler.h, Os.h, Os_Cfg.h, Rte_*.h, SWC header)
#   2. SOME/IP library public headers
# ---------------------------------------------------------------------------
target_include_directories(BulkDataTransfer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${SOMEIP_ROOT}/include
)

# ---------------------------------------------------------------------------
# Link against the SOME/IP static library
# ---------------------------------------------------------------------------
target_link_libraries(BulkDataTransfer
    PRIVATE
        someip
)

# ---------------------------------------------------------------------------
# Compiler standards and warnings
# ---------------------------------------------------------------------------
target_compile_features(BulkDataTransfer PRIVATE c_std_99)

target_compile_options(BulkDataTransfer
    PRIVATE
        -Wall
        -Wextra
        -Wcast-align
        -Wmissing-prototypes
        -Wstrict-prototypes
        -Wno-unused-parameter   # OS hook stubs take unused params
)

# ---------------------------------------------------------------------------
# Optional: install target (copies binary to CMAKE_INSTALL_BINDIR)
# ---------------------------------------------------------------------------
install(TARGETS BulkDataTransfer
        RUNTIME DESTINATION bin)

# ---------------------------------------------------------------------------
# Print configuration summary
# ---------------------------------------------------------------------------
message(STATUS "----------------------------------------------------")
message(STATUS " BulkDataTransfer example application")
message(STATUS " Build type  : ${CMAKE_BUILD_TYPE}")
message(STATUS " SOMEIP_ROOT : ${SOMEIP_ROOT}")
message(STATUS " Compiler    : ${CMAKE_C_COMPILER}")
message(STATUS "----------------------------------------------------")
