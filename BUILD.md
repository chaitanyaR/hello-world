# Build Guide

**Project:** SOME/IP AUTOSAR Middleware + VehicleNetwork + BulkDataTransfer  
**AUTOSAR Release:** R22-11  
**Toolchain:** GCC (host Linux/macOS) · MinGW-w64 / MSYS2 (host Windows) · arm-none-eabi-gcc (embedded)

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Repository Layout](#2-repository-layout)
3. [Quick Start (Host Build – Linux)](#3-quick-start-host-build)
4. [Windows Build – Virtual Ethernet Demo](#4-windows-build--virtual-ethernet-demo)
5. [Docker / Container Packaging](#5-docker--container-packaging)
6. [Detailed Build Steps](#6-detailed-build-steps)
7. [Build Targets](#7-build-targets)
8. [Running the Test Suite (SWE.5 / SWE.6)](#8-running-the-test-suite-swe5--swe6)
9. [Code Coverage](#9-code-coverage)
10. [Cross-Compilation for Embedded (arm-none-eabi)](#10-cross-compilation-for-embedded-arm-none-eabi)
11. [Aurix TC34xx MCAL Layer](#11-aurix-tc34xx-mcal-layer)
12. [Static Analysis](#12-static-analysis)
13. [Dependency Graph](#13-dependency-graph)
14. [Troubleshooting](#14-troubleshooting)

---

## 1. Prerequisites

### Mandatory (all platforms)

| Tool | Minimum Version | Purpose |
|------|----------------|---------|
| `cmake` | 3.16 | Build system |
| `make` or `ninja` | any | Build backend |
| `git` | 2.30 | Source control |

### Linux / macOS

| Tool | Minimum Version | Purpose |
|------|----------------|---------|
| `gcc` / `g++` | 11.0 | Host compilation (C99 + C++17) |

```bash
sudo apt-get install cmake gcc g++ make git   # Debian/Ubuntu
brew install cmake gcc make git               # macOS
```

### Windows (MSYS2 / MinGW-w64) — recommended

MinGW-w64 is the recommended Windows toolchain.  It provides GCC + Winsock2
with no additional license requirements.

1. **Install MSYS2**: download from <https://www.msys2.org/> and run the installer.
2. Open the **MSYS2 MINGW64** terminal and install the toolchain:

```bash
pacman -S --needed \
    mingw-w64-x86_64-toolchain \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-ninja \
    git
```

3. Add `C:\msys64\mingw64\bin` to your Windows `PATH`.

> **Alternative – Visual Studio / MSVC**  
> Install [CMake for Windows](https://cmake.org/download/) and Visual Studio 2022
> with the "Desktop development with C++" workload.  Pass `-G "Visual Studio 17 2022"`
> to cmake.  MSVC 19.30+ is required for C99 `_Bool` / `<stdbool.h>`.

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

## 3. Quick Start (Host Build – Linux)

```bash
# 1. Clone
git clone https://github.com/chaitanyaR/hello-world.git
cd hello-world

# 2. Build SOME/IP library + GTest suite
cmake -S someip -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

# 3. Run tests
ctest --test-dir build --output-on-failure

# 4. Build VehicleNetwork 5-node SOME/IP demo (includes TC34xx MCAL stub)
cmake -S examples/VehicleNetwork -B build_vehicle \
      -DCMAKE_BUILD_TYPE=Release \
      -DSOMEIP_ROOT=${PWD}/someip
cmake --build build_vehicle --parallel

# 5. Run the virtual Ethernet demo
bash examples/VehicleNetwork/launch_network.sh build_vehicle
```

---

## 4. Windows Build – Virtual Ethernet Demo

The VehicleNetwork example runs on Windows using Winsock2 for UDP loopback
sockets.  The platform abstraction layer (`Platform.h`) automatically selects
Winsock2 vs. POSIX at compile time – no source changes required.

### 4.1 Prerequisites (Windows)

Install MSYS2 with MinGW-w64 as described in [Section 1](#1-prerequisites).

### 4.2 Build (MSYS2 MINGW64 terminal)

```bash
# Open MSYS2 MINGW64 terminal
git clone https://github.com/chaitanyaR/hello-world.git
cd hello-world

# Configure (GCC via MinGW; Ninja generator is optional but faster)
cmake -S examples/VehicleNetwork \
      -B build_vehicle_win \
      -G "MinGW Makefiles" \
      -DCMAKE_BUILD_TYPE=Release \
      -DSOMEIP_ROOT=${PWD}/someip \
      -DSOMEIP_BUILD_TESTS=OFF

# Build all 5 ECU nodes + MCAL TC34xx stub
cmake --build build_vehicle_win --parallel
```

### 4.3 Build (Visual Studio / MSVC — x64 Developer Command Prompt)

```cmd
cmake -S examples\VehicleNetwork ^
      -B build_vehicle_win ^
      -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DSOMEIP_ROOT=%CD%\someip ^
      -DSOMEIP_BUILD_TESTS=OFF

cmake --build build_vehicle_win --config Release
```

### 4.4 Run the virtual Ethernet simulation

#### CMD (batch script)

```cmd
cd examples\VehicleNetwork
launch_network.bat ..\..\build_vehicle_win\Release
```

#### PowerShell

```powershell
cd examples\VehicleNetwork
.\launch_network.ps1 -BuildDir ..\..\build_vehicle_win\Release
```

The script:
1. Starts all 5 node processes concurrently (BCM, ECM, ADAS, Gateway, IPC)
2. Each node binds to its UDP port on `127.0.0.1` (Windows loopback)
3. SOME/IP-SD frames are exchanged via Winsock2 `sendto` / `recvfrom`
4. After ~12 s all nodes shut down and logs are printed in colour

**Expected output (PowerShell, abridged):**

```
[BCM] Phase 1: Offering body services...
[ECM] Phase 1: Offering powertrain services...
[ADAS] EngineStatus found! Subscribing to eventgroup 0x0001...
[IPC] │  DoorLock             : SUBSCRIBED         │
[GW]  Routing Table (10 entries)
[IPC] │  DoorLock             : UNAVAILABLE        │
```

### 4.5 What the platform abstraction does on Windows

| Linux (POSIX)               | Windows (Winsock2 / Win32)         |
|-----------------------------|------------------------------------|
| `#include <sys/socket.h>`   | `#include <winsock2.h>`            |
| `int` socket fd             | `SOCKET` (unsigned ptr)            |
| `pthread_create()`          | `CreateThread()` (Win32)           |
| `usleep(N)` / `sleep(N)`    | `Sleep(ms)`                        |
| `close(sock)`               | `closesocket(sock)`                |
| `select(fd+1, ...)`         | `select(0, ...)` (nfds ignored)   |
| no init required            | `WSAStartup()` / `WSACleanup()`   |

All differences are hidden behind `Platform.h` in
`examples/VehicleNetwork/platform/`.  The five node mains and
`NodeTransport.c` include only `Platform.h` and are otherwise unchanged.

---

## 5. Docker / Container Packaging

Docker is the recommended way to share the SOME/IP development environment
without requiring recipients to install CMake, GCC, or any AUTOSAR tooling.

### Concept and trade-offs

| Approach | Pros | Cons |
|----------|------|------|
| **Runtime image** (5 nodes, 1 container) | Simplest; loopback works out of the box | Doesn't model network isolation between ECUs |
| **Multi-container** (1 container per ECU) | Each ECU has its own IP – realistic topology | Requires peer IP injection via env vars |
| **SDK image** (dev environment) | Reproduces the build environment exactly; works in GitHub Codespaces | Larger image (~500 MB with clang-tidy/valgrind) |

> **Honest critique of single-fat-container approach**: running all 5 node
> processes inside one container shares the same network namespace.  That is
> fine for a demo but does not model real ECU isolation.  For integration
> testing use the multi-container compose file, which gives each container a
> real IP address on a Docker bridge network.
>
> **On realism**: Docker bridge networking is still software-defined loopback –
> it is not genuine Ethernet.  For that you need real hardware or QEMU with
> a TAP bridge.  Docker is however the right tool for developer onboarding and
> CI pipeline execution.

### 5.1 Build Docker images

```bash
# Slim runtime image (~30 MB) – runs the 5-node simulation
docker build --target runtime -t someip-vehicle:runtime .

# Full SDK image (~500 MB) – all build tools + pre-built libs
docker build --target sdk -t someip-vehicle:sdk .
```

### 5.2 Run the demo (single container)

```bash
# All 5 ECU nodes start inside one container on 127.0.0.1
docker run --rm -it someip-vehicle:runtime
```

Or with Docker Compose (auto-builds if no image exists):

```bash
cd examples/VehicleNetwork
docker compose up --build
```

Expected output (abridged, ~9 s runtime):
```
[BCM] Phase 1: Offering body services...
[ECM] Phase 1: Offering powertrain services...
[IPC] │  DoorLock : SUBSCRIBED │
[GW]  Routing Table (10 entries)
Network simulation complete.
```

### 5.3 Run the multi-container simulation (realistic ECU isolation)

Each ECU gets its own IP on a 172.20.0.0/24 Docker bridge:

```bash
cd examples/VehicleNetwork
docker compose -f docker-compose.multinode.yml up --build
```

Follow logs per node:
```bash
docker compose -f docker-compose.multinode.yml logs -f bcm
docker compose -f docker-compose.multinode.yml logs -f ipc
```

Peer addresses are injected via `TRANSPORT_PEER_HOSTS` (comma-separated IPs).
NodeTransport resolves each address with `getaddrinfo()`, so Docker Compose
service names (DNS) work just as well as fixed IPs.

### 5.4 SDK developer image (VS Code / Codespaces)

Open the repository in VS Code with the Dev Containers extension – it reads
`.devcontainer/devcontainer.json` and builds the `sdk` target automatically.

```bash
# Or manually: open a shell in the SDK container
docker run --rm -it -v "$(pwd):/workspace" someip-vehicle:sdk

# Inside the container – build your own app against the pre-installed SOME/IP
cmake -S /opt/someip-sdk/examples/VehicleNetwork -B /tmp/demo \
      -DSOMEIP_ROOT=${SOMEIP_ROOT}
cmake --build /tmp/demo --parallel
bash /opt/someip-sdk/examples/VehicleNetwork/launch_network.sh /tmp/demo
```

`SOMEIP_ROOT` is pre-set to `/opt/someip-sdk/someip` in the SDK image.

### 5.5 Dockerfile structure

```
Dockerfile (repo root)
├── Stage: builder   – ubuntu:24.04 + cmake + ninja + gcc; compiles everything
├── Stage: runtime   – copies 5 node binaries + launch script only (~30 MB)
└── Stage: sdk       – based on builder + gdb + valgrind + clang-tidy + cppcheck
```

### 5.6 Publishing to GitHub Container Registry (optional)

```bash
docker login ghcr.io -u <github-username>

docker tag someip-vehicle:runtime ghcr.io/<owner>/someip-vehicle:runtime
docker tag someip-vehicle:sdk     ghcr.io/<owner>/someip-vehicle:sdk

docker push ghcr.io/<owner>/someip-vehicle:runtime
docker push ghcr.io/<owner>/someip-vehicle:sdk
```

Once pushed, anyone can run the demo without cloning the repository:
```bash
docker run --rm -it ghcr.io/<owner>/someip-vehicle:runtime
```

---

## 6. Detailed Build Steps (Linux)

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

## 7. Build Targets

| Target | Binary | Description |
|--------|--------|-------------|
| `someip` | `libsomeip.a` | SOME/IP static library |
| `someip_tests` | `someip_tests` | SWE.5/SWE.6 GTest executable |
| `BulkDataTransfer` | `BulkDataTransfer` | Example application |

---

## 8. Running the Test Suite (SWE.5 / SWE.6)

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

## 9. Code Coverage

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

## 10. Cross-Compilation for Embedded (arm-none-eabi)

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

## 11. Aurix TC34xx MCAL Layer

The `mcal/tc34xx/` sub-tree provides AUTOSAR R22-11 compliant MCAL stubs for
the Infineon Aurix TC344/TC347/TC348 family.  All hardware register accesses
are guarded by `ETH_17_GETH_MAC_HOST_SIM` so the same source compiles both
for the real silicon (TriCore toolchain) and for the Linux/Windows host
simulation (GCC/MSVC).

### Modules

| Module | Header | Source | AUTOSAR SWS |
|--------|--------|--------|-------------|
| Ethernet MAC | `Eth_17_GEthMac.h` | `Eth_17_GEthMac.c` | SWS_Eth |
| Ethernet Interface | `EthIf.h` | `EthIf.c` | SWS_EthIf |
| Port (RMII pin-mux) | `Port_17_GtmCcu6.h` | *(stubs in header)* | SWS_Port |
| MCU (clock tree) | `Mcu.h` | *(stubs in header)* | SWS_Mcu |
| EQOS register map | `IfxEth_regdef.h` | — | (Infineon MCAL) |
| Pre-compile config | `Eth_17_GEthMac_Cfg.h` | — | SWS_Eth cfg |

### Build with MCAL enabled (default)

```bash
cmake -S examples/VehicleNetwork -B build_vehicle \
      -DSOMEIP_ROOT=${PWD}/someip \
      -DVEHICLE_BUILD_MCAL_TC34XX=ON    # (default)
cmake --build build_vehicle --parallel
```

This produces `build_vehicle/libmcal_tc34xx.a` alongside the five ECU executables.

### Build without MCAL

```bash
cmake -S examples/VehicleNetwork -B build_vehicle \
      -DSOMEIP_ROOT=${PWD}/someip \
      -DVEHICLE_BUILD_MCAL_TC34XX=OFF
```

### Host-simulation vs. real silicon

| Feature | HOST_SIM (default) | Real TC34xx |
|---------|--------------------|-------------|
| Register access | no-ops (printf only) | MCAL_REG32_WRITE / _READ |
| TX path | NodeTransport UDP | EQOS DMA descriptor ring |
| RX path | NodeTransport thread | Eth_17_GEthMac_Receive() → EthIf_RxIndication |
| PHY init | logged, no MDIO | TJA1100 via MDIO/MDCC |
| Clock init | no-ops | Mcu_InitClock() → PLL0 |
| Toolchain | GCC / MSVC | TriCore compiler (Aurix TC toolchain) |

---

## 12. Static Analysis

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

## 13. Dependency Graph

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

## 14. Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `FetchContent` fails on first configure | No internet access | Pre-stage GTest (see §1) |
| `arm-none-eabi-gcc: not found` | Toolchain not installed | `sudo apt-get install gcc-arm-none-eabi` |
| `undefined reference to SomeIpSd_Transmit` | Integration stub not linked | Link `stub_transport.c` or provide a BSW `SomeIpSd_Transmit` |
| DET error count > 0 in tests | Module not initialised | Call `SomeIp_Init(NULL)` and `SomeIpSd_Init(NULL)` before use |
| `lcov: ERROR` on coverage | Missing `--coverage` flags | Use `build_cov` configuration above |
| Linker: multiple definition of `Det_*` | Both `Det.c` and test stub included | Link only one; `Det.c` is the authoritative stub |
