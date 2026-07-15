# SDV HPC + Zonal ECU Platform

## Overview

This platform implements a Software Defined Vehicle (SDV) architecture in which a central
High-Performance Computer (HPC) runs ADAS (Advanced Driver-Assistance Systems) and AD
(Autonomous Driving) features. Four Zonal ECUs handle domain-specific work and communicate
with the HPC over **SOME/IP** (ISO/IEC 20077). Diagnostics are exposed via **SOVD**
(Service Oriented Vehicle Diagnostics, ISO 17879) over HTTP/REST.

```
┌─────────────────────────────────────────────────────┐
│                      HPC                            │
│  ┌─────────────┐  ┌──────────┐  ┌───────────────┐  │
│  │ ADAS        │  │ AD Stack │  │ SOVD Server   │  │
│  │ Pipeline    │  │ (L4)     │  │ /api/sovd/v1/ │  │
│  │ 50 Hz       │  │ EKF +    │  │               │  │
│  │ ASIL-D      │  │ Spline   │  │ ZoneForwarder │  │
│  └──────┬──────┘  └────┬─────┘  └───────┬───────┘  │
└─────────┼──────────────┼────────────────┼───────────┘
          │   SOME/IP    │                │ REST → SOME/IP
   ┌──────┴──────────────┴────────────────┴──────┐
   │              SOME/IP Service Discovery       │
   └───┬───────────────┬──────────────┬───────┬───┘
       │               │              │       │
┌──────┴──────┐ ┌──────┴──────┐ ┌────┴────┐ ┌┴────────────┐
│ Powertrain  │ │ ADAS Zone   │ │Telemetry│ │ Body ECU    │
│ ECU         │ │ ECU         │ │ ECU     │ │             │
│ TorqueArb.  │ │ RadarDriver │ │ OtaMgr  │ │ Lighting    │
│ BatteryMgr  │ │ LidarDriver │ │ ConnMgr │ │ HVAC        │
│ RegenBrake  │ │ SafetyGW    │ │ V2xGW   │ │ DoorWindow  │
│ 0x1001      │ │ 0x1002      │ │ 0x1003  │ │ 0x1004      │
└─────────────┘ └─────────────┘ └─────────┘ └─────────────┘
```

---

## SOME/IP Service Map

| Service              | Service ID | Instance | Event Group |
|----------------------|-----------|----------|-------------|
| PowertrainService    | `0x1001`  | `0x0001` | `0x0011`    |
| AdasZoneService      | `0x1002`  | `0x0001` | `0x0012`    |
| TelemetryService     | `0x1003`  | `0x0001` | `0x0013`    |
| BodyService          | `0x1004`  | `0x0001` | `0x0014`    |
| DiagnosticService    | `0x1FFF`  | `0x0001` | `0x001F`    |

---

## SOVD Resource Tree

Base URL: `http://<host>:8080/api/sovd/v1/`

```
/api/sovd/v1/
├── hpc/
│   ├── data/       adas-pipeline-state, ad-stack-state, cpu-load
│   ├── faults/     DTC read / clear
│   └── routines/   self-test, calibrate-sensors
└── zones/
    ├── powertrain/
    │   ├── data/     battery-soc, torque-actual, regen-mode
    │   ├── faults/
    │   └── routines/ actuator-test, battery-conditioning
    ├── adas/
    │   ├── data/     sensor-health, safety-state, actuator-positions
    │   ├── faults/
    │   └── routines/ sensor-calibration, actuator-plausibility-check
    ├── telemetry/
    │   ├── data/     connectivity-status, ota-status, gnss-position
    │   └── routines/ trigger-ota, connectivity-test
    └── body/
        ├── data/     lighting-states, door-states, hvac-state
        └── routines/ lighting-test, door-actuator-test
```

All endpoints require `Authorization: Bearer <token>`. Async routines return `202 Accepted`
with a `Location` header for status polling.

---

## Key Design Patterns

| Pattern | Component | Purpose |
|---------|-----------|---------|
| Safety Gateway | `SafetyGateway` wraps `ActuatorController` | ISO 26262 ASIL-D plausibility + rate-limit interlock; holds last valid command on violation |
| Pipeline | `AdasPipeline` stage graph | SensorFusion → Detector → Tracker → Planner → Decision; each stage independently testable |
| State Machine | `BatteryManager`, `OtaManager` | Explicit FSM: IDLE/CHARGING/DISCHARGING/FAULT; IDLE/DOWNLOADING/VERIFYING/INSTALLING/DONE |
| Observer | SOME/IP broadcasts | Decoupled event delivery via `ServiceRegistry::subscribeAvailability` |
| Facade | `ZoneForwarder` | Translates SOVD REST calls for `/zones/*` into `DiagnosticService` SOME/IP calls |
| Command | `ActuatorCommand` value object | Auditable, replayable command flowing through SafetyGateway → ActuatorController |

---

## Build Instructions

### Native (Ubuntu 24.04)

```bash
# Install dependencies
sudo apt-get install -y cmake ninja-build gcc g++ \
    libgtest-dev nlohmann-json3-dev doxygen graphviz

# Build (debug, with tests)
cmake --preset=debug
cmake --build build/debug --parallel $(nproc)
ctest --preset=unit --output-on-failure

# Generate this documentation
cmake --build build/debug --target docs
# Open build/docs/html/index.html

# Release build
cmake --preset=release
cmake --build build/release --parallel $(nproc)
```

### Docker

```bash
# Build image (compiles, tests, and generates docs)
docker build -t sdv-platform .

# Serve documentation on http://localhost:8080
docker run -p 8080:8080 sdv-platform

# Extract release binaries
docker create --name sdv-tmp sdv-platform
docker cp sdv-tmp:/app/bin ./bin
docker rm sdv-tmp
```

---

## Directory Layout

```
hello-world/
├── CMakeLists.txt          # Root CMake (C++20, presets, dependencies)
├── CMakePresets.json       # debug / release / simulation presets
├── Dockerfile              # Multi-stage: build → test → docs → serve
├── cmake/
│   ├── CompilerOptions.cmake
│   ├── DocTargets.cmake    # Doxygen 'docs' target
│   └── CodeCoverage.cmake
├── docs/
│   ├── Doxyfile            # Doxygen configuration
│   └── architecture.md     # This file (Doxygen mainpage)
├── interfaces/
│   └── service_definitions/  # Franca IDL (.fidl) contracts
├── middleware/
│   ├── someip/             # SOME/IP service registry, proxy, stub
│   └── sovd/               # ISO 17879 SOVD HTTP server
├── src/
│   ├── hpc/
│   │   ├── adas/           # 50 Hz ADAS pipeline (SensorFusion → Decision)
│   │   ├── ad/             # AD stack (EKF Localizer, Spline Trajectory, Behavior FSM)
│   │   └── sovd_server/    # SOVD resource handlers + ZoneForwarder
│   └── zones/
│       ├── powertrain/     # TorqueArbiter, BatteryManager FSM, RegenBraking
│       ├── adas_zone/      # Radar/Lidar/Camera drivers, SafetyGateway
│       ├── telemetry/      # OtaManager FSM, ConnectivityManager, V2xGateway
│       └── body/           # LightingController, HvacController, DoorWindowController
└── tests/
    ├── unit/               # 45 GoogleTest unit tests
    ├── integration/        # In-process full-pipeline integration tests
    └── mocks/              # GMock implementations
```

---

## Test Summary

| Suite | Count | Command |
|-------|-------|---------|
| Unit tests | 45 | `ctest --preset=unit --output-on-failure` |
| Integration tests | 5 | `ctest --preset=integration --output-on-failure` |
| Coverage report | — | `cmake --build build/debug --target coverage` |
