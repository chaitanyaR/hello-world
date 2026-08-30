@echo off
REM ============================================================
REM  launch_network.bat – Virtual Ethernet Network Demo (Windows)
REM  AUTOSAR R22-11  |  5-Node SOME/IP Service Discovery
REM
REM  Usage:
REM    launch_network.bat [BUILD_DIR]
REM
REM  Example (from the VehicleNetwork source directory):
REM    cmake -S . -B build_vehicle_win -DSOMEIP_ROOT=..\..\someip
REM    cmake --build build_vehicle_win --config Release
REM    launch_network.bat build_vehicle_win\Release
REM
REM  Each node runs in its own CMD window.
REM  Logs are written to node_logs\*.log in the current directory.
REM  Close any node window or press Ctrl-C in this window to stop all.
REM ============================================================

setlocal enabledelayedexpansion

REM --- Resolve build directory -------------------------------------------
if "%~1"=="" (
    set "BDIR=build_vehicle_win\Release"
) else (
    set "BDIR=%~1"
)

REM Normalise to absolute path
for %%F in ("%BDIR%") do set "BDIR=%%~fF"

echo [NET] Build directory: %BDIR%

REM --- Verify executables exist ------------------------------------------
for %%N in (bcm_node ecm_node adas_node gateway_node ipc_node) do (
    if not exist "%BDIR%\%%N.exe" (
        echo [NET] ERROR: %BDIR%\%%N.exe not found.
        echo [NET] Build with:
        echo [NET]   cmake -S . -B build_vehicle_win -DSOMEIP_ROOT=..\..\someip
        echo [NET]   cmake --build build_vehicle_win --config Release
        exit /b 1
    )
)

REM --- Create log directory -----------------------------------------------
if not exist "node_logs" mkdir node_logs

echo.
echo [NET] ============================================================
echo [NET]  Virtual Ethernet Network  ^|  SOME/IP Service Discovery
echo [NET]  5 Nodes: BCM  ECM  ADAS  Gateway  IPC
echo [NET]  Transport: Windows loopback  127.0.0.1
echo [NET]  Ports:     BCM=30501  ECM=30502  ADAS=30503
echo [NET]             GW=30504   IPC=30505
echo [NET] ============================================================
echo.
echo [NET] Starting nodes (each in its own window)...

REM --- Launch each node in a separate window ------------------------------
REM  /MIN  = start minimised
REM  /WAIT is NOT used so all 5 start concurrently

start "BCM  Node" /MIN cmd /c "%BDIR%\bcm_node.exe"     > "node_logs\bcm.log"  2>&1
start "ECM  Node" /MIN cmd /c "%BDIR%\ecm_node.exe"     > "node_logs\ecm.log"  2>&1
start "ADAS Node" /MIN cmd /c "%BDIR%\adas_node.exe"    > "node_logs\adas.log" 2>&1
start "GW   Node" /MIN cmd /c "%BDIR%\gateway_node.exe" > "node_logs\gw.log"   2>&1
start "IPC  Node" /MIN cmd /c "%BDIR%\ipc_node.exe"     > "node_logs\ipc.log"  2>&1

echo [NET] All 5 nodes launched.  Logs: node_logs\*.log
echo [NET] Waiting ~12 s for the simulation to complete...

REM --- Wait for simulation duration (nodes run ~9 s each) -----------------
timeout /t 12 /nobreak > nul

echo.
echo [NET] ============================================================
echo [NET]  Simulation complete.  Final logs:
echo [NET] ============================================================

REM --- Print a summary of each log ---------------------------------------
for %%N in (bcm ecm adas gw ipc) do (
    echo.
    echo --- node_logs\%%N.log ---
    type "node_logs\%%N.log"
)

echo.
echo [NET] Done.
endlocal
