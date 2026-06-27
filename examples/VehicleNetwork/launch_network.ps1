#Requires -Version 5.1
<#
.SYNOPSIS
    Virtual Ethernet Network Demo – Windows PowerShell launcher
    AUTOSAR R22-11 | 5-Node SOME/IP Service Discovery

.DESCRIPTION
    Builds (optional) and launches all five ECU node processes simultaneously,
    each writing its output to node_logs\<name>.log.
    After ~12 s the simulation completes and logs are printed in colour.

.PARAMETER BuildDir
    Path to the CMake build output directory containing the .exe files.
    Default: "build_vehicle_win\Release"

.EXAMPLE
    # Build then run
    cmake -S . -B build_vehicle_win -DSOMEIP_ROOT=..\..\someip
    cmake --build build_vehicle_win --config Release
    .\launch_network.ps1 -BuildDir build_vehicle_win\Release

.EXAMPLE
    # Run with default build directory
    .\launch_network.ps1
#>

param(
    [string]$BuildDir = "build_vehicle_win\Release"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ---- Resolve and validate build directory ---------------------------------
$BuildDir = Resolve-Path $BuildDir -ErrorAction SilentlyContinue

if (-not $BuildDir) {
    Write-Host "[NET] ERROR: Build directory '$($PSBoundParameters.BuildDir)' not found." -ForegroundColor Red
    Write-Host "[NET] Build with:"
    Write-Host "[NET]   cmake -S . -B build_vehicle_win -DSOMEIP_ROOT=..\..\someip"
    Write-Host "[NET]   cmake --build build_vehicle_win --config Release"
    exit 1
}

$nodes = @(
    @{ Name = "bcm_node";     Tag = "BCM    "; Color = "Cyan"    },
    @{ Name = "ecm_node";     Tag = "ECM    "; Color = "Green"   },
    @{ Name = "adas_node";    Tag = "ADAS   "; Color = "Yellow"  },
    @{ Name = "gateway_node"; Tag = "Gateway"; Color = "Magenta" },
    @{ Name = "ipc_node";     Tag = "IPC    "; Color = "White"   }
)

# ---- Verify executables ---------------------------------------------------
foreach ($n in $nodes) {
    $exe = Join-Path $BuildDir "$($n.Name).exe"
    if (-not (Test-Path $exe)) {
        Write-Host "[NET] ERROR: $exe not found." -ForegroundColor Red
        exit 1
    }
}

# ---- Create log directory -------------------------------------------------
New-Item -ItemType Directory -Force -Path "node_logs" | Out-Null

# ---- Banner ---------------------------------------------------------------
Write-Host ""
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "  Virtual Ethernet Network  |  SOME/IP Service Discovery   " -ForegroundColor Cyan
Write-Host "  5 Nodes: BCM  ECM  ADAS  Gateway  IPC                    " -ForegroundColor Cyan
Write-Host "  Transport: Windows loopback  127.0.0.1                   " -ForegroundColor Cyan
Write-Host "  Ports: BCM=30501  ECM=30502  ADAS=30503                  " -ForegroundColor Cyan
Write-Host "         GW=30504   IPC=30505                               " -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""

# ---- Launch all nodes simultaneously -------------------------------------
$processes = @()

foreach ($n in $nodes) {
    $exe  = Join-Path $BuildDir "$($n.Name).exe"
    $log  = "node_logs\$($n.Name -replace '_node','').log"
    Write-Host "[NET] Starting $($n.Tag.Trim()) -> $exe" -ForegroundColor $n.Color
    $proc = Start-Process -FilePath $exe `
                          -RedirectStandardOutput $log `
                          -RedirectStandardError  "$log.err" `
                          -NoNewWindow -PassThru
    $processes += $proc
}

Write-Host ""
Write-Host "[NET] All 5 nodes running.  Logs: node_logs\*.log" -ForegroundColor Gray
Write-Host "[NET] Waiting ~12 s for simulation to complete..." -ForegroundColor Gray

# ---- Wait for all processes (max 15 s) ------------------------------------
$deadline = (Get-Date).AddSeconds(15)

foreach ($proc in $processes) {
    $remaining = [int]($deadline - (Get-Date)).TotalSeconds
    if ($remaining -gt 0) {
        $proc.WaitForExit($remaining * 1000) | Out-Null
    }
    if (-not $proc.HasExited) {
        $proc.Kill()
    }
}

# ---- Print logs with colour tags -----------------------------------------
Write-Host ""
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "  Simulation complete – node output"                          -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan

$logFiles = @(
    @{ File = "node_logs\bcm.log";         Tag = "BCM    "; Color = "Cyan"    },
    @{ File = "node_logs\ecm.log";         Tag = "ECM    "; Color = "Green"   },
    @{ File = "node_logs\adas.log";        Tag = "ADAS   "; Color = "Yellow"  },
    @{ File = "node_logs\gateway_node.log"; Tag = "Gateway"; Color = "Magenta" },
    @{ File = "node_logs\ipc_node.log";    Tag = "IPC    "; Color = "White"   }
)

foreach ($lf in $logFiles) {
    Write-Host ""
    Write-Host "--- $($lf.File) ---" -ForegroundColor $lf.Color
    if (Test-Path $lf.File) {
        Get-Content $lf.File | ForEach-Object {
            Write-Host "  $_" -ForegroundColor $lf.Color
        }
    } else {
        Write-Host "  (no output)" -ForegroundColor DarkGray
    }
}

Write-Host ""
Write-Host "[NET] Done." -ForegroundColor Green
