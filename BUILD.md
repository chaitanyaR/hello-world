# Build Guide

**Project:** SOME/IP AUTOSAR Middleware + BulkDataTransfer Example Application  
**AUTOSAR Release:** R22-11  
**Toolchain:** GCC (host) / arm-none-eabi-gcc (target)

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Repository Layout](#2-repository-layout)
3. [Quick Start (Host Build)](#3-quick-start-host-build)
4. [Detailed Build Steps](#4-detailed-build-steps)
5. [Build Targets](#5-build-targets)
6. [Running the Test Suite (SWE.5 / SWE.6)](#6-running-the-test-suite-swe5--swe6)
7. [Code Coverage](#7-code-coverage)
8. [Cross-Compilation for Embedded (arm-none-eabi)](#8-cross-compilation-for-embedded-arm-none-eabi)
9. [Static Analysis](#9-static-analysis)
10. [Dependency Graph](#10-dependency-graph)
11. [Troubleshooting](#11-troubleshooting)

---

## 1. Prerequisites

### Mandatory

| Tool | Minimum Version | Purpose |
|------|----------------|---------|
| `cmake` | 3.16 | Build system |
| `gcc` / `g++` | 11.0 | Host compilation (C99 + C++17) |
| `make` or `ninja` | any | Build backend |
| `git` | 2.30 | Source control |

### For GTest (auto-fetched, no manual install needed)

CMake `FetchContent` downloads Google Test v1.14.0 automatically on first
configure if `SOMEIP_BUILD_TESTS=ON` (the default).  An internet connection
is required on the first build; subsequent builds use the CMake cache.

To **pre-stage** GTest for offline builds:

```bash
# Download once
curl -L https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz \
     -o /opt/gtest/v1.14.0.tar.gz

# Point CMake to the local archive
cmake -S someip -B build \
      -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/opt/gtest/v1.14.0.tar.gz
```

### For Code Coverage (optional)

```bash
sudo apt-get install lcov   # Debian/Ubuntu
brew install lcov           # macOS
```

### For Cross-Compilation (optional)

```bash
sudo apt-get install gcc-arm-none-eabi binutils-arm-none-eabi
```

### For Static Analysis (optional)

```bash
sudo apt-get install cppcheck clang-tidy
```

---

## 2. Repository Layout

```
hello-world/
├── BUILD.md                         ← This file
├── .gitignore
├── someip/                          ← SOME/IP AUTOSAR middleware library
│   ├── CMakeLists.txt               ← Top-level CMake (builds library + tests)
│   ├── include/
│   │   ├── Std_Types.h              ← AUTOSAR standard types
│   │   ├── ComStack_Types.h         ← AUTOSAR comm-stack types (PduInfoType …)
│   │   ├── Det.h                    ← Default Error Tracer API
│   │   ├── SomeIp_Types.h           ← SOME/IP type definitions
│   │   ├── SomeIp_Cfg.h             ← Pre-compile configuration
│   │   ├── SomeIp.h                 ← Transformer module API
│   │   ├── SomeIp_Cbk.h             ← Callback declarations
│   │   └── SomeIp_SD.h              ← Service Discovery API
│   ├── src/
│   │   ├── SomeIp.c                 ← Transformer implementation
│   │   ├── SomeIp_SD.c              ← Service Discovery implementation
│   │   └── Det.c                    ← DET stub
│   ├── tests/
│   │   ├── CMakeLists.txt           ← GTest build
│   │   ├── stub_transport.c/.h      ← SomeIpSd_Transmit() test stub
│   │   ├── test_someip_serializer.cpp ← SWE.5 serializer unit tests
│   │   └── test_someip_sd.cpp         ← SWE.5 SD unit tests
│   └── docs/
│       ├── architecture.md          ← Architecture + Mermaid diagrams
│       ├── swe1_requirements.md     ← SWE.1 Software Requirements
│       ├── swe2_architectural_design.md
│       ├── swe5_unit_verification.md
│       └── swe6_qualification_test.md
└── examples/
    └── BulkDataTransfer/            ← Example AUTOSAR application
        ├── arxml/
        │   ├── BulkDataTransfer_SWC.arxml  ← SWC + port interface description
        │   └── OsConfig.arxml              ← OS ECUC configuration
        ├── include/
        │   ├── Compiler.h           ← AUTOSAR compiler abstraction
        │   ├── Os.h                 ← AUTOSAR OS API
        │   ├── Os_Cfg.h             ← Generated OS configuration
        │   ├── Rte_BulkDataTransfer.h ← Generated RTE header
        │   └── BulkDataTransfer.h   ← SWC interface
        ├── src/
        │   ├── main.c               ← ECU main / BSW init sequence
        │   ├── Os_Cfg.c             ← Generated OS task/alarm definitions
        │   ├── Rte_BulkDataTransfer.c ← RTE stub implementation
        │   └── BulkDataTransfer.c   ← SWC implementation
        └── CMakeLists.txt
```

---

## 3. Quick Start (Host Build)

```bash
# 1. Clone
git clone https://github.com/chaitanyaR/hello-world.git
cd hello-world

# 2. Build SOME/IP library + GTest suite
cmake -S someip -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

# 3. Run tests
ctest --test-dir build --output-on-failure

# 4. Build BulkDataTransfer example
cmake -S examples/BulkDataTransfer -B build_example \
      -DCMAKE_BUILD_TYPE=Debug \
      -DSOMEIP_ROOT=${PWD}/someip
cmake --build build_example --parallel
```

---

## 4. Detailed Build Steps

### 4.1 SOME/IP Library

```bash
# Configure (Debug build, tests enabled by default)
cmake -S someip -B build \
      -DCMAKE_BUILD_TYPE=Debug \
      -DSOMEIP_BUILD_TESTS=ON

# Build
cmake --build build --parallel $(nproc)

# Artifacts
ls build/libsomeip.a          # Static library
ls build/tests/someip_tests   # GTest executable
```

**CMake options:**

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Debug` | `Debug` / `Release` / `RelWithDebInfo` |
| `SOMEIP_BUILD_TESTS` | `ON` | Build GTest suite |
| `SOMEIP_DEV_ERROR_DETECT` | (from `SomeIp_Cfg.h`) | Override DET compile switch |

### 4.2 BulkDataTransfer Example Application

```bash
cmake -S examples/BulkDataTransfer -B build_example \
      -DCMAKE_BUILD_TYPE=Debug \
      -DSOMEIP_ROOT=${PWD}/someip

cmake --build build_example --parallel

# Run the host simulation
./build_example/BulkDataTransfer
```

### 4.3 Combined (library + example) from one root

Create a top-level `CMakeLists.txt` if needed:

```bash
cmake -S . -B build_all \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_EXAMPLES=ON
cmake --build build_all --parallel
```

---

## 5. Build Targets

| Target | Binary | Description |
|--------|--------|-------------|
| `someip` | `libsomeip.a` | SOME/IP static library |
| `someip_tests` | `someip_tests` | SWE.5/SWE.6 GTest executable |
| `BulkDataTransfer` | `BulkDataTransfer` | Example application |

---

## 6. Running the Test Suite (SWE.5 / SWE.6)

```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run with verbose output
ctest --test-dir build -V

# Run only serializer tests
ctest --test-dir build -R "SWE5_SER"

# Run only SD tests
ctest --test-dir build -R "SWE5_SD"

# Run with JUnit XML output (for CI)
ctest --test-dir build \
      --output-junit test_results.xml \
      --output-on-failure
```

Expected output (all green):

```
Test project /path/to/build
    Start 1: SerializerTest.SWE5_SER_001_...
1/35 Test #1: SerializerTest.SWE5_SER_001_... Passed  0.001s
...
35/35 Test  : NonFunctionalTest.SWE5_NF_...   Passed  0.001s

100% tests passed, 0 tests failed out of 35
```

---

## 7. Code Coverage

```bash
# Configure with coverage flags
cmake -S someip -B build_cov \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS="--coverage -O0" \
      -DCMAKE_CXX_FLAGS="--coverage -O0" \
      -DCMAKE_EXE_LINKER_FLAGS="--coverage"

cmake --build build_cov --parallel

# Run tests to generate .gcda files
ctest --test-dir build_cov

# Generate HTML coverage report
lcov --capture \
     --directory build_cov \
     --output-file coverage.info \
     --exclude "*/tests/*" \
     --exclude "/usr/*"

genhtml coverage.info \
        --output-directory coverage_html \
        --title "SOME/IP SWE.5 Coverage"

# Open report
xdg-open coverage_html/index.html   # Linux
open     coverage_html/index.html   # macOS
```

Coverage targets (per SWE.5):

| Module | Statement | Branch |
|--------|-----------|--------|
| `SomeIp.c` | ≥ 100% | ≥ 100% |
| `SomeIp_SD.c` | ≥ 100% | ≥ 100% |

---

## 8. Cross-Compilation for Embedded (arm-none-eabi)

### 8.1 Toolchain file (`cmake/arm-none-eabi.cmake`)

```cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR           arm-none-eabi-ar)
set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
set(CMAKE_SIZE         arm-none-eabi-size)

# Cortex-M4 example – adjust for your MCU
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS_INIT   "${CPU_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections -specs=nano.specs")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

### 8.2 Cross-compile commands

```bash
# Library only (no GTest for target)
cmake -S someip -B build_arm \
      -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DSOMEIP_BUILD_TESTS=OFF

cmake --build build_arm --parallel

# Example application
cmake -S examples/BulkDataTransfer -B build_arm_example \
      -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DSOMEIP_ROOT=${PWD}/someip

cmake --build build_arm_example --parallel

# Inspect binary size
arm-none-eabi-size build_arm/libsomeip.a
arm-none-eabi-size build_arm_example/BulkDataTransfer.elf
```

### 8.3 Flash (ST-LINK example)

```bash
arm-none-eabi-objcopy -O ihex \
    build_arm_example/BulkDataTransfer.elf \
    BulkDataTransfer.hex

st-flash write BulkDataTransfer.hex 0x08000000
```

---

## 9. Static Analysis

### cppcheck

```bash
cppcheck \
    --enable=all \
    --suppress=missingIncludeSystem \
    --std=c99 \
    --platform=native \
    -I someip/include \
    someip/src/ \
    examples/BulkDataTransfer/src/ \
    2>&1 | tee cppcheck.log

# Zero findings at severity "error" required for ASPICE Level 2
grep "error" cppcheck.log | wc -l
```

### clang-tidy

```bash
# Generate compile_commands.json
cmake -S someip -B build_tidy \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DSOMEIP_BUILD_TESTS=OFF

clang-tidy \
    -p build_tidy \
    -checks="cert-*,misra-*,clang-analyzer-*" \
    someip/src/SomeIp.c \
    someip/src/SomeIp_SD.c
```

---

## 10. Dependency Graph

```
BulkDataTransfer (executable)
├── BulkDataTransfer.c          ← SWC implementation
├── Rte_BulkDataTransfer.c      ← Generated RTE stub
├── Os_Cfg.c                    ← Generated OS configuration
├── main.c                      ← ECU main / BSW init
└── libsomeip.a                 ← SOME/IP middleware
    ├── SomeIp.c                ← Transformer (serialize/deserialize)
    ├── SomeIp_SD.c             ← Service Discovery
    └── Det.c                   ← Default Error Tracer stub

External headers (no .c / no link dependency):
  Std_Types.h, ComStack_Types.h  ← AUTOSAR base types
  Os.h                           ← AUTOSAR OS API (target: OEM RTOS)
  Compiler.h                     ← AUTOSAR compiler abstraction
```

---

## 11. Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `FetchContent` fails on first configure | No internet access | Pre-stage GTest (see §1) |
| `arm-none-eabi-gcc: not found` | Toolchain not installed | `sudo apt-get install gcc-arm-none-eabi` |
| `undefined reference to SomeIpSd_Transmit` | Integration stub not linked | Link `stub_transport.c` or provide a BSW `SomeIpSd_Transmit` |
| DET error count > 0 in tests | Module not initialised | Call `SomeIp_Init(NULL)` and `SomeIpSd_Init(NULL)` before use |
| `lcov: ERROR` on coverage | Missing `--coverage` flags | Use `build_cov` configuration above |
| Linker: multiple definition of `Det_*` | Both `Det.c` and test stub included | Link only one; `Det.c` is the authoritative stub |
