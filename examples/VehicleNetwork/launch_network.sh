#!/usr/bin/env bash
# launch_network.sh – Start all 5 vehicle ECU nodes simultaneously
#
# Usage:
#   bash launch_network.sh [BUILD_DIR]
#
# Default BUILD_DIR: build_vehicle
#
# The script launches all 5 node processes in parallel, captures their
# combined output to node_logs/<name>.log, and waits for all to finish.
# Ctrl-C terminates all nodes cleanly.

set -euo pipefail

BUILD_DIR="${1:-build_vehicle}"

if [ ! -d "$BUILD_DIR" ]; then
    echo "ERROR: build directory '$BUILD_DIR' not found."
    echo "  Build first: cmake -S examples/VehicleNetwork -B $BUILD_DIR -DSOMEIP_ROOT=\${PWD}/someip"
    echo "               cmake --build $BUILD_DIR --parallel"
    exit 1
fi

for exe in bcm_node ecm_node adas_node gateway_node ipc_node hpc_node; do
    if [ ! -x "$BUILD_DIR/$exe" ]; then
        echo "ERROR: $BUILD_DIR/$exe not found or not executable."
        exit 1
    fi
done

LOG_DIR="node_logs"
mkdir -p "$LOG_DIR"

echo "============================================================"
echo " Vehicle SOME/IP Network – 5 ECU nodes"
echo "============================================================"
echo " Logs: $LOG_DIR/<node>.log"
echo " SD ports: BCM=30501 ECM=30502 ADAS=30503 GW=30504 IPC=30505"
echo " HPC SOVD: UDP 30506  |  HTTP dashboard: http://localhost:8080/dashboard"
echo " Transport: UDP loopback (127.0.0.1)"
echo "============================================================"
echo ""
echo "Launching nodes..."
echo ""

# Track child PIDs for clean shutdown
PIDS=()

cleanup() {
    echo ""
    echo "--- Interrupt received – stopping all nodes ---"
    for pid in "${PIDS[@]}"; do
        kill "$pid" 2>/dev/null || true
    done
    wait
    echo "--- All nodes stopped ---"
}
trap cleanup INT TERM

# Start each node, redirecting to its log file and also tee-ing to stdout
# with a coloured prefix so interleaved output is readable.

run_node() {
    local name=$1
    local exe=$2
    local colour=$3
    "$BUILD_DIR/$exe" 2>&1 | \
        while IFS= read -r line; do
            printf '\e[%sm%s\e[0m\n' "$colour" "$line"
        done | tee "$LOG_DIR/${name}.log"
}

# Node colour codes (ANSI):  32=green 33=yellow 34=blue 35=magenta 36=cyan 37=white
run_node BCM     bcm_node     "32" &  PIDS+=($!)
run_node ECM     ecm_node     "33" &  PIDS+=($!)
run_node ADAS    adas_node    "34" &  PIDS+=($!)
run_node GW      gateway_node "35" &  PIDS+=($!)
run_node IPC     ipc_node     "36" &  PIDS+=($!)
run_node HPC     hpc_node     "37" &  PIDS+=($!)

echo "All 6 nodes started (PIDs: ${PIDS[*]})"
echo "  Dashboard: http://localhost:8080/dashboard"
echo "  REST API:  http://localhost:8080/sovd/v1/"
echo "Waiting for completion (~15 seconds)..."
echo ""

wait

echo ""
echo "============================================================"
echo " Network simulation complete."
echo " Log files: $LOG_DIR/"
echo "============================================================"
