# User Manual
## AUTOSAR SOME/IP Vehicle Network — 5-Node ECU Simulation

**Version:** 1.0  
**AUTOSAR Release:** R22-11  
**Platforms:** Linux · Windows 10/11 · macOS · Docker

---

## Table of Contents

1. [What This Application Does](#1-what-this-application-does)
2. [Network Architecture](#2-network-architecture)
3. [Service Catalogue](#3-service-catalogue)
4. [Quick Start — Docker (recommended)](#4-quick-start--docker-recommended)
5. [Quick Start — Linux / macOS](#5-quick-start--linux--macos)
6. [Quick Start — Windows](#6-quick-start--windows)
7. [Understanding the Output](#7-understanding-the-output)
8. [Test Scenarios and Pass Criteria](#8-test-scenarios-and-pass-criteria)
9. [Running the Unit Test Suite](#9-running-the-unit-test-suite)
10. [Troubleshooting](#10-troubleshooting)
11. [Writing Your Own SOME/IP Node](#11-writing-your-own-someip-node)
12. [Reference: SOME/IP Service IDs](#12-reference-someip-service-ids)

---

## 1. What This Application Does

This application simulates a 5-ECU in-vehicle Ethernet network where every ECU
communicates using the **SOME/IP (Scalable service-Oriented Middleware over IP)**
protocol as defined by AUTOSAR Release R22-11.

**What you observe when you run it:**

- Five software processes start simultaneously, each representing one Electronic
  Control Unit (ECU) in a vehicle.
- The ECUs discover each other's services automatically using **SOME/IP Service
  Discovery (SOME/IP-SD)** — the same mechanism used in production vehicles.
- Each ECU advertises the services it provides (`OfferService`), searches for
  services it needs (`FindService`), and subscribes to event notifications
  (`SubscribeEventgroup`).
- After ~9 seconds every ECU shuts down gracefully, sending `StopOffer` and
  `StopSubscribe` messages to its peers.
- The Domain Gateway builds a live **service routing table** and prints it during
  the run — useful for understanding which services are currently active on the
  network.

**What this is NOT:**

- The ECUs do not send actual vehicle sensor data (RPM, door positions, etc.) —
  only the service discovery handshake is simulated.
- There is no CAN bus, FlexRay, or hardware Ethernet — transport is UDP over the
  OS loopback (`127.0.0.1`) or Docker bridge network.
- The Aurix TC34xx MCAL layer compiles and initialises, but all hardware register
  accesses are stubs (host-simulation mode, `ETH_17_GETH_MAC_HOST_SIM`).

---

## 2. Network Architecture

```
                    ┌──────────────────────────────────────┐
                    │   Virtual 100BASE-T1 Ethernet Switch  │
                    │      (UDP loopback / Docker bridge)   │
                    └──┬──────┬──────┬──────┬──────────────┘
                       │      │      │      │
              ┌────────┘  ┌───┘  ┌───┘  ┌───┘  ┌──────┐
              │           │      │      │      │      │
         ┌────▼───┐  ┌────▼──┐ ┌─▼────┐ ┌▼────┐ ┌────▼──┐
         │  BCM   │  │  ECM  │ │ ADAS │ │  GW │ │  IPC  │
         │:30501  │  │:30502 │ │:30503│ │:30504│ │:30505 │
         └────────┘  └───────┘ └──────┘ └─────┘ └───────┘
         Provider    Provider   Both    Monitor  Consumer
```

| ECU | Role | Description |
|-----|------|-------------|
| **BCM** | Provider | Body Control Module — door locks, windows, lights |
| **ECM** | Provider | Engine Control Module — engine status, throttle |
| **ADAS** | Provider + Consumer | ADAS Controller — lane keeping, collision warning; needs engine status |
| **GW** | Monitor + Provider | Domain Gateway — service routing table, cross-domain proxy |
| **IPC** | Consumer | Instrument Cluster — displays door lock, engine, ADAS status |

All five processes communicate over UDP. In the single-container or
single-machine mode they use `127.0.0.1`. In the multi-container Docker mode
each container has a dedicated IP on `172.20.0.0/24`.

---

## 3. Service Catalogue

| Service | ID | Instance | Provider | Consumers |
|---------|----|----------|----------|-----------|
| DoorLock | `0x0101` | `0x0001` | BCM | IPC, GW |
| WindowControl | `0x0102` | `0x0001` | BCM | GW |
| LightControl | `0x0103` | `0x0001` | BCM | GW |
| EngineStatus | `0x0201` | `0x0001` | ECM | ADAS, IPC, GW |
| ThrottleControl | `0x0202` | `0x0001` | ECM | GW |
| LaneKeeping | `0x0301` | `0x0001` | ADAS | IPC, GW |
| CollisionWarning | `0x0302` | `0x0001` | ADAS | GW |
| GatewayRouting | `0x0401` | `0x0001` | GW | (discovery only) |

All services use Major Version `1`, Minor Version `0`, TTL `60` seconds.

---

## 4. Quick Start — Docker (recommended)

Docker requires no local compiler or toolchain installation.

### Prerequisites

- Docker Engine 20.10+ or Docker Desktop 4.0+
- `docker compose` (included with Docker Desktop; on Linux: `sudo apt install docker-compose-plugin`)

Verify installation:

```bash
docker --version        # Docker version 24.x or later
docker compose version  # Docker Compose version v2.x
```

### Option A — Single container (simplest)

All 5 ECU processes run inside one container, communicating over the
container's internal loopback (`127.0.0.1`).

```bash
# Clone the repository
git clone https://github.com/chaitanyaR/hello-world.git
cd hello-world

# Build the image and run the simulation (~2 minutes first time)
docker compose -f examples/VehicleNetwork/docker-compose.yml up --build
```

Expected: coloured output from all 5 nodes for ~9 seconds, then
"Network simulation complete."

To skip rebuilding on subsequent runs:

```bash
docker compose -f examples/VehicleNetwork/docker-compose.yml up
```

### Option B — Multi-container (realistic ECU isolation)

Each ECU runs in its own container on a `172.20.0.0/24` Docker bridge network.
Every container has a separate IP address, modelling real network isolation.

```bash
cd hello-world
docker compose -f examples/VehicleNetwork/docker-compose.multinode.yml up --build
```

Follow a single node's log:

```bash
docker compose -f examples/VehicleNetwork/docker-compose.multinode.yml logs -f bcm
docker compose -f examples/VehicleNetwork/docker-compose.multinode.yml logs -f ipc
```

### Option C — SDK container (development shell)

Use the SDK image to experiment with the SOME/IP library in an isolated
environment without installing anything on your host.

```bash
# Build the SDK image (includes gdb, valgrind, clang-tidy, cppcheck)
docker build --target sdk -t someip-vehicle:sdk .

# Open a development shell; your project directory is mounted at /workspace
docker run --rm -it -v "$(pwd):/workspace" someip-vehicle:sdk

# Inside the container — build and run the demo manually
cmake -S /opt/someip-sdk/examples/VehicleNetwork \
      -B /tmp/demo \
      -DSOMEIP_ROOT=${SOMEIP_ROOT}
cmake --build /tmp/demo --parallel
bash /opt/someip-sdk/examples/VehicleNetwork/launch_network.sh /tmp/demo
```

---

## 5. Quick Start — Linux / macOS

### Prerequisites

```bash
# Debian / Ubuntu
sudo apt-get install cmake gcc g++ make git

# macOS (Homebrew)
brew install cmake gcc make git
```

### Build and run

```bash
# Clone
git clone https://github.com/chaitanyaR/hello-world.git
cd hello-world

# Configure
cmake -S examples/VehicleNetwork \
      -B build_vehicle \
      -DCMAKE_BUILD_TYPE=Release \
      -DSOMEIP_ROOT="${PWD}/someip"

# Build (produces 5 node executables + libmcal_tc34xx.a)
cmake --build build_vehicle --parallel

# Run the 5-node simulation (~9 seconds)
bash examples/VehicleNetwork/launch_network.sh build_vehicle
```

Logs are written to `node_logs/` in the current directory.  
Press **Ctrl-C** at any time to stop all nodes cleanly.

---

## 6. Quick Start — Windows

### Prerequisites

Install **MSYS2** from <https://www.msys2.org/>, then open the **MSYS2 MINGW64**
terminal:

```bash
pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake git
```

### Build (MSYS2 MINGW64 terminal)

```bash
git clone https://github.com/chaitanyaR/hello-world.git
cd hello-world

cmake -S examples/VehicleNetwork \
      -B build_vehicle_win \
      -G "MinGW Makefiles" \
      -DCMAKE_BUILD_TYPE=Release \
      -DSOMEIP_ROOT="${PWD}/someip" \
      -DSOMEIP_BUILD_TESTS=OFF

cmake --build build_vehicle_win --parallel
```

### Run — PowerShell (recommended)

```powershell
cd examples\VehicleNetwork
.\launch_network.ps1 -BuildDir ..\..\build_vehicle_win
```

### Run — CMD batch script

```cmd
cd examples\VehicleNetwork
launch_network.bat ..\..\build_vehicle_win
```

The PowerShell script shows each ECU's output in a different colour.
The CMD script writes logs to `node_logs\` and prints them after completion.

---

## 7. Understanding the Output

### Log prefix convention

Every log line is prefixed with the ECU name in square brackets:

```
[BCM]   Body Control Module
[ECM]   Engine Control Module
[ADAS]  ADAS Controller
[GW]    Domain Gateway
[IPC]   Instrument Panel Cluster
```

Secondary tags on Gateway lines indicate the log category:

```
[GW] [RT]   Routing table update (service added / removed)
[GW] [SD]   Raw SD event log (Find / Subscribe passthrough)
```

### Annotated run output

Below is a full annotated run. Actual order may vary slightly due to
thread scheduling; the events always converge to the same final state.

```
==========================================================
 BCM (Body Control Module) – SOME/IP Node
 ECU ID: 0x01  |  SD Port: 30501
==========================================================
```
> BCM binds UDP port 30501. Every node prints its ECU ID and port on startup.

```
[BCM] Phase 1: Offering body services...
[BCM] -> OfferService: DoorLock (0x0101)
[BCM] -> OfferService: WindowControl (0x0102)
[BCM] -> OfferService: LightControl (0x0103)
```
> BCM broadcasts three `OfferService` SD entries to all peers. Peers who need
> these services will respond with `SubscribeEventgroup`.

```
[GW] [RT] Offer: DoorLock (0x0101/0x0001) v1 TTL=60s
[GW] [RT] Offer: WindowControl (0x0102/0x0001) v1 TTL=60s
[GW] [RT] Offer: LightControl (0x0103/0x0001) v1 TTL=60s
```
> Gateway's routing table callback fires for every offer it receives. `[RT]`
> means the routing table was updated (entry added or TTL refreshed).

```
[ECM] Phase 1: Offering powertrain services...
[ECM] -> OfferService: EngineStatus (0x0201)
[ECM] -> OfferService: ThrottleControl (0x0202)
```
> ECM offers its two services. ADAS and IPC are waiting for EngineStatus.

```
[ADAS] Phase 2: Finding EngineStatus (needed for ADAS enable)...
[ADAS] -> FindService: EngineStatus (0x0201)
```
> ADAS sends a `FindService` SD frame. ECM receives it and re-offers.

```
[ECM] <- Find: EngineStatus (0x0201/0x0001) v1
[ECM] -> Re-offering EngineStatus in response to FIND
```
> ECM sees the FIND and re-announces EngineStatus with a fresh TTL so ADAS
> can hear it even if the initial Offer arrived before ADAS started.

```
[ADAS]    EngineStatus found! Subscribing to eventgroup 0x0001...
[ADAS] -> SubscribeEventgroup: EngineStatus EG=0x0001
[ECM] <- Subscribe: EngineStatus (0x0201/0x0001) EG=0x0001 TTL=60s
[ECM]    Subscriber added – will send EngineStatus events
```
> The full SOME/IP-SD subscribe handshake:
> 1. ADAS sends `SubscribeEventgroup`
> 2. ECM receives it and logs the subscriber

```
[IPC] Phase 2: Finding required vehicle services...
[IPC] -> FindService: EngineStatus (0x0201)
[IPC] -> FindService: DoorLock (0x0101)
[IPC] -> FindService: LaneKeeping (0x0301)
```
> IPC (pure consumer) searches for the three services it needs to display on
> the instrument cluster.

```
[IPC] <- Offer: EngineStatus (0x0201/0x0001) v1 TTL=60s
[IPC] -> SubscribeEventgroup: EngineStatus EG=0x0001
[IPC] ┌─ Cluster Display Update ─────────────────┐
[IPC] │  EngineStatus        : SUBSCRIBED         │
[IPC] └─────────────────────────────────────────────┘
```
> IPC responds to the incoming Offer by subscribing and updating its display
> widget to SUBSCRIBED.

```
[GW] ┌─ Routing Table (10 entries) ────────────────────────────┐
[GW] │  DoorLock              0x0101/0x0001  v1  TTL= 60s  UP
[GW] │  WindowControl         0x0102/0x0001  v1  TTL= 60s  UP
[GW] │  LightControl          0x0103/0x0001  v1  TTL= 60s  UP
[GW] │  EngineStatus          0x0201/0x0001  v1  TTL= 60s  UP
[GW] │  ThrottleControl       0x0202/0x0001  v1  TTL= 60s  UP
[GW] │  LaneKeeping           0x0301/0x0001  v1  TTL= 60s  UP
[GW] │  CollisionWarning      0x0302/0x0001  v1  TTL= 60s  UP
[GW] │  GatewayRouting        0x0401/0x0001  v1  TTL= 60s  UP
[GW] └────────────────────────────────────────────────────────┘
```
> The Gateway's routing table snapshot — printed ~5 seconds into the run.
> All 8 services (plus re-offer duplicates) should be UP.

```
[BCM] Phase 4: StopOffer all services...
[BCM] -> StopOfferService: DoorLock
[IPC] <- StopOffer: DoorLock (0x0101/0x0001)
[IPC] ┌─ Cluster Display Update ─────────────────┐
[IPC] │  DoorLock            : UNAVAILABLE        │
[IPC] └─────────────────────────────────────────────┘
```
> Graceful shutdown: BCM sends `StopOffer` (TTL=0) and IPC's display widget
> updates to UNAVAILABLE as the service disappears.

```
[IPC] Phase 4: Stopping subscriptions...
[IPC] -> StopSubscribeEventgroup: EngineStatus
[IPC] -> StopSubscribeEventgroup: DoorLock
[IPC] -> StopSubscribeEventgroup: LaneKeeping
[IPC] Shutdown complete.
```
> IPC unsubscribes from all eventgroups before exiting — correct SOME/IP-SD
> shutdown behaviour.

```
============================================================
 Network simulation complete.
 Log files: node_logs/
============================================================
```
> All 5 nodes have exited cleanly. Exit code 0.

---

## 8. Test Scenarios and Pass Criteria

Run through the following checklist after each execution to verify the
application is working correctly.

### TS-01 — Node startup and transport binding

**How to verify:** Check that all 5 nodes print their banner and bind message.

**Pass criteria:**
```
[Transport] bound to UDP port 30501  (4 peer(s))   ← BCM
[Transport] bound to UDP port 30502  (4 peer(s))   ← ECM
[Transport] bound to UDP port 30503  (4 peer(s))   ← ADAS
[Transport] bound to UDP port 30504  (4 peer(s))   ← Gateway
[Transport] bound to UDP port 30505  (4 peer(s))   ← IPC
```

**Fail indicator:** `ERROR: transport init failed` — port already in use (see
[Troubleshooting](#10-troubleshooting)).

---

### TS-02 — Service offer propagation

**How to verify:** Every `OfferService` sent by a provider must appear in at
least one peer's log.

**Pass criteria (minimum):**
- `[ECM] <- Offer: DoorLock` — ECM sees BCM's offer ✓
- `[GW] [RT] Offer: EngineStatus` — Gateway records ECM's offer ✓
- `[IPC] <- Offer: LaneKeeping` — IPC sees ADAS's offer ✓

Check: the Gateway routing table printed at ~5 seconds must contain **8 UP
entries** (one per service).

---

### TS-03 — FindService / re-offer handshake

**How to verify:** ADAS and IPC send `FindService`; the provider re-offers.

**Pass criteria:**
```
[ADAS] -> FindService: EngineStatus (0x0201)
[ECM] <- Find: EngineStatus (0x0201/0x0001) v1
[ECM] -> Re-offering EngineStatus in response to FIND
[ADAS] <- Offer: EngineStatus (0x0201/0x0001) v1 TTL=60s
```

---

### TS-04 — EventGroup subscription

**How to verify:** Each consumer subscribes after receiving an offer; the
provider acknowledges.

**Pass criteria:**

| Subscriber | Service | Expected provider log |
|---|---|---|
| ADAS | EngineStatus | `[ECM] <- Subscribe: EngineStatus … EG=0x0001 TTL=60s` |
| IPC | EngineStatus | `[ECM] <- Subscribe: EngineStatus … EG=0x0001 TTL=60s` |
| IPC | DoorLock | `[BCM] <- Subscribe: DoorLock … EG=0x0001 TTL=60s` |
| IPC | LaneKeeping | `[ADAS] <- Subscribe: LaneKeeping … EG=0x0001 TTL=60s` |

The IPC display must show `SUBSCRIBED` for each of the three services.

---

### TS-05 — IPC cluster display updates

**How to verify:** IPC prints display widgets for all three subscribed services.

**Pass criteria (all three must appear):**
```
[IPC] │  EngineStatus        : SUBSCRIBED         │
[IPC] │  DoorLock            : SUBSCRIBED         │
[IPC] │  LaneKeeping         : SUBSCRIBED         │
```

---

### TS-06 — Gateway routing table completeness

**How to verify:** The routing table printed by GW must contain all services.

**Pass criteria:** Table has **≥ 8 entries**, all showing **UP**.

Services that must be UP: DoorLock, WindowControl, LightControl, EngineStatus,
ThrottleControl, LaneKeeping, CollisionWarning, GatewayRouting.

---

### TS-07 — Graceful shutdown (StopOffer / StopSubscribe)

**How to verify:** Every provider sends StopOffer and every consumer
sends StopSubscribe before exiting.

**Pass criteria:**
```
[BCM] -> StopOfferService: DoorLock
[BCM] -> StopOfferService: WindowControl
[BCM] -> StopOfferService: LightControl
[ECM] -> StopOfferService: EngineStatus
[ECM] -> StopOfferService: ThrottleControl
[ADAS] -> StopOfferService: LaneKeeping
[ADAS] -> StopOfferService: CollisionWarning
[ADAS] -> StopSubscribeEventgroup: EngineStatus
[IPC]  -> StopSubscribeEventgroup: EngineStatus
[IPC]  -> StopSubscribeEventgroup: DoorLock
[IPC]  -> StopSubscribeEventgroup: LaneKeeping
[GW]   -> StopOfferService: GatewayRouting
```

The IPC display must update the stopped services to `UNAVAILABLE`.

---

### TS-08 — Clean exit (all nodes exit 0)

**How to verify:** The launch script prints the completion banner.

**Pass criteria:**
```
============================================================
 Network simulation complete.
 Log files: node_logs/
============================================================
```
No node may print `ERROR` or exit with a non-zero code.

---

### TS-09 — MCAL TC34xx stub initialisation

**How to verify:** The MCAL Ethernet driver initialises in host-simulation mode.

This is exercised when you link `mcal_tc34xx` into your own application.
To verify from the build alone:

```bash
# Check the library exists and is non-empty
ls -lh build_vehicle/libmcal_tc34xx.a

# Symbols must include the key MCAL functions
nm build_vehicle/libmcal_tc34xx.a | grep "Eth_17_GEthMac_Init\|EthIf_Init"
```

**Pass criteria:** `libmcal_tc34xx.a` exists and `nm` shows `T Eth_17_GEthMac_Init`
and `T EthIf_Init`.

---

### TS-10 — Multi-container isolation (Docker only)

**How to verify:** Run the multi-container compose and confirm IPs are distinct.

```bash
docker compose -f examples/VehicleNetwork/docker-compose.multinode.yml up -d --build
docker inspect ecu_bcm  | grep '"IPAddress"'
docker inspect ecu_ipc  | grep '"IPAddress"'
```

**Pass criteria:**
- `ecu_bcm` has IP `172.20.0.10`
- `ecu_ipc` has IP `172.20.0.14`
- Each node's log shows `[Transport] bound to UDP port 3050X (4 peer(s))`

Clean up:
```bash
docker compose -f examples/VehicleNetwork/docker-compose.multinode.yml down
```

---

## 9. Running the Unit Test Suite

The SOME/IP library includes 35 GTest unit tests covering the serialiser and
Service Discovery state machine.

```bash
# Configure with tests enabled (requires internet for first run – downloads GTest)
cmake -S someip -B build_tests \
      -DCMAKE_BUILD_TYPE=Debug \
      -DSOMEIP_BUILD_TESTS=ON

cmake --build build_tests --parallel

# Run all tests
ctest --test-dir build_tests --output-on-failure

# Run only the serialiser tests
ctest --test-dir build_tests -R "SWE5_SER"

# Run only the SD state machine tests
ctest --test-dir build_tests -R "SWE5_SD"

# Generate JUnit XML report (for CI pipelines)
ctest --test-dir build_tests \
      --output-junit test_results.xml \
      --output-on-failure
```

**Expected result:**
```
100% tests passed, 0 tests failed out of 35
```

### Offline GTest installation

If the build machine has no internet access:

```bash
curl -L https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz \
     -o /tmp/gtest.tar.gz

cmake -S someip -B build_tests \
      -DSOMEIP_BUILD_TESTS=ON \
      -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/tmp/gtest.tar.gz
```

---

## 10. Troubleshooting

### "ERROR: transport init failed" / "bind: Address already in use"

**Cause:** A previous run left a node process running, or another application
holds one of the ports 30501–30505.

**Fix:**
```bash
# Linux / macOS – find and kill the leftover process
lsof -i UDP:30501-30505
kill <PID>

# Windows PowerShell
Get-Process | Where-Object { $_.MainWindowTitle -match "ECU" } | Stop-Process
```

---

### Launch script exits immediately with "ERROR: not found or not executable"

**Cause:** The build directory path passed to the script is wrong, or the build
did not complete successfully.

**Fix:** Re-run the cmake build and verify the executables exist:
```bash
ls build_vehicle/bcm_node build_vehicle/ecm_node \
   build_vehicle/adas_node build_vehicle/gateway_node build_vehicle/ipc_node
```

---

### Only 2–3 nodes print output; some nodes appear silent

**Cause:** Race condition during startup — fast nodes sent their initial
`OfferService` before slow nodes had finished binding.

**Explanation:** This is expected behaviour. SOME/IP-SD's `FindService` /
re-offer mechanism resolves it: when IPC sends `FindService`, providers that
have already offered re-announce their service so late starters receive it.
All test scenarios should still pass.

**If the problem persists:** increase the startup delay by setting
`VEHICLE_STARTUP_DELAY_MS=1000` and rebuilding (requires a short code change
in `main()` — change `Platform_SleepMs(500u)` to `Platform_SleepMs(1000u)`).

---

### Docker: "Cannot connect to the Docker daemon"

**Fix:**
```bash
# Linux – start the Docker service
sudo systemctl start docker
sudo usermod -aG docker $USER   # add yourself to the docker group
newgrp docker                   # apply group change without logging out
```

---

### Docker multi-container: nodes start but produce no output

**Cause:** In detached mode (`-d`) logs are not printed to the terminal.

**Fix:** Run without `-d`, or follow logs explicitly:
```bash
docker compose -f examples/VehicleNetwork/docker-compose.multinode.yml logs -f
```

---

### Windows PowerShell: "execution of scripts is disabled"

**Fix:**
```powershell
# Allow scripts for the current user only
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

---

### Gateway routing table shows fewer than 8 entries

**Cause:** Some nodes started later than the Gateway's 5-second monitoring
window; their offers were not received in time to populate the table.

**Explanation:** This is a timing artefact of the demo. In production, TTL
refresh and `FindService` re-announcements ensure eventual consistency.

**Verification:** Check the individual node logs — all 8 services should appear
in GW's `[RT]` lines even if the final snapshot shows fewer.

---

### "FetchContent" fails on cmake configure (no internet)

**Fix:** Pre-stage GTest locally (see [Section 9](#9-running-the-unit-test-suite)).

---

## 11. Writing Your Own SOME/IP Node

Use the SDK container or the built library to add a new ECU node.

### Step 1 — Copy a template node

```bash
cp examples/VehicleNetwork/nodes/bcm_main.c examples/VehicleNetwork/nodes/my_ecu_main.c
```

### Step 2 — Assign a port and peers

Edit `VehicleServices.h` to add your node's port:

```c
#define NODE_PORT_MY_ECU   ((uint16)30506u)
```

### Step 3 — Implement the node

Your node's `main()` follows this pattern:

```c
int main(void)
{
    /* 1. List the ports of all peer nodes */
    const uint16 peers[] = { NODE_PORT_BCM, NODE_PORT_ECM, /* ... */ };

    /* 2. Register callbacks for SD events you care about */
    NodeSdCallbacks cbs = {
        .OnOfferService    = MyEcu_OnOfferService,
        .OnFindService     = NULL,   /* NULL = ignore */
        .OnSubscribeEventgroup    = NULL,
        .OnSubscribeEventgroupAck = MyEcu_OnSubscribeAck
    };

    /* 3. Init BSW */
    SomeIp_Init(NULL_PTR);
    SomeIpSd_Init(NULL_PTR);

    /* 4. Init transport */
    NodeTransport_Init(NODE_PORT_MY_ECU, peers,
                       (uint8)(sizeof(peers)/sizeof(peers[0])));
    NodeTransport_RegisterSdCallbacks(&cbs);
    NodeTransport_StartReceive();

    /* 5. Wait for peers */
    Platform_SleepMs(500u);

    /* 6. Offer your services */
    SomeIpSd_OfferService(MY_SERVICE_ID, MY_INSTANCE_ID,
                          MY_MAJOR, MY_MINOR, SOMEIPSD_TTL_DEFAULT);

    /* 7. Find services you need */
    SomeIpSd_FindService(SOMEIP_SVC_ENGINE_STATUS, SOMEIP_SVC_ENGINE_STATUS_INST,
                         SOMEIP_SVC_ENGINE_STATUS_MAJOR, SOMEIP_SVC_ENGINE_STATUS_MINOR);

    /* 8. Run */
    Platform_SleepMs(7000u);

    /* 9. Shutdown */
    SomeIpSd_StopOfferService(MY_SERVICE_ID, MY_INSTANCE_ID,
                               MY_MAJOR, MY_MINOR);
    NodeTransport_Deinit();
    return 0;
}
```

### Step 4 — Add to CMakeLists.txt

In `examples/VehicleNetwork/CMakeLists.txt`, add one line after the existing
node definitions:

```cmake
add_node_executable(my_ecu_node   nodes/my_ecu_main.c)
```

### Step 5 — Build and run

```bash
cmake --build build_vehicle --parallel
./build_vehicle/my_ecu_node
```

### Key SOME/IP-SD API functions

| Function | Description |
|----------|-------------|
| `SomeIpSd_OfferService(svcId, instId, major, minor, ttl)` | Advertise a service; peers receive `OnOfferService` callback |
| `SomeIpSd_StopOfferService(svcId, instId, major, minor)` | Withdraw a service (sends TTL=0); peers receive `OnOfferService` with `ttl=0` |
| `SomeIpSd_FindService(svcId, instId, major, minor)` | Broadcast a service request; providers re-offer if they match |
| `SomeIpSd_SubscribeEventgroup(svcId, instId, egId, major, ttl)` | Subscribe to an eventgroup on a provider |
| `SomeIpSd_StopSubscribeEventgroup(svcId, instId, egId, major)` | Unsubscribe (sends TTL=0) |

Use `SOMEIP_INSTANCE_ID_ANY`, `SOMEIP_MAJOR_VERSION_ANY`, and
`SOMEIP_MINOR_VERSION_ANY` in `FindService` to match any instance or version.

---

## 12. Reference: SOME/IP Service IDs

| Constant | Value | Description |
|----------|-------|-------------|
| `SOMEIP_SVC_DOOR_LOCK` | `0x0101` | Door lock / unlock |
| `SOMEIP_SVC_WINDOW_CTRL` | `0x0102` | Window raise / lower |
| `SOMEIP_SVC_LIGHT_CTRL` | `0x0103` | Exterior lighting |
| `SOMEIP_SVC_ENGINE_STATUS` | `0x0201` | Engine RPM, temperature, state |
| `SOMEIP_SVC_THROTTLE_CTRL` | `0x0202` | Throttle position |
| `SOMEIP_SVC_LANE_KEEPING` | `0x0301` | Lane departure warning / assist |
| `SOMEIP_SVC_COLLISION_WARN` | `0x0302` | Forward collision warning |
| `SOMEIP_SVC_GW_ROUTING` | `0x0401` | Cross-domain service proxy |
| `SOMEIP_SD_SERVICE_ID` | `0xFFFF` | SOME/IP-SD (internal) |

### UDP port assignments

| ECU | Port | Environment variable for Docker override |
|-----|------|----------------------------------------|
| BCM | 30501 | `TRANSPORT_PEER_HOSTS=<peer-ip-list>` |
| ECM | 30502 | (same env var, different container) |
| ADAS | 30503 | |
| Gateway | 30504 | |
| IPC | 30505 | |

`TRANSPORT_PEER_HOSTS` accepts a comma-separated list of IP addresses or
hostnames, one per peer, in the order the node's `main()` passes them to
`NodeTransport_Init()`. When unset, all peers default to `127.0.0.1`.
