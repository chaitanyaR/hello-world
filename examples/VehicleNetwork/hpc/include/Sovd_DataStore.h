/**
 * Sovd_DataStore.h  –  Thread-Safe Vehicle Data Store for SOVD HPC
 *
 * Centralises all live ECU data that the SOVD HTTP server exposes.  Data
 * flows in from two sources:
 *   1. SOME/IP receive callbacks (NodeTransport background thread)
 *   2. Cyclic simulation tick (Sovd_DataStore_Tick) – fills gaps where no
 *      SOME/IP provider is present (TCU, extended ADAS signals).
 *
 * All public functions are thread-safe: callers must NOT hold the lock
 * themselves; internal locking is handled inside each function.
 */

#ifndef SOVD_DATASTORE_H
#define SOVD_DATASTORE_H

#include "Std_Types.h"
#include "Sovd_Types.h"

/* =========================================================================
 * Platform mutex (pthread on POSIX, CRITICAL_SECTION on Windows)
 * ====================================================================== */
#ifdef _WIN32
#  include <windows.h>
   typedef CRITICAL_SECTION Sovd_MutexType;
#else
#  include <pthread.h>
   typedef pthread_mutex_t  Sovd_MutexType;
#endif

/* =========================================================================
 * Per-domain data structures
 * ====================================================================== */

/** ADAS – High Performance Computer domain */
typedef struct {
    uint8   lane_departure_active;  /* 0 = clear, 1 = departure detected  */
    uint8   collision_warn_level;   /* 0=clear  1=caution 2=warn 3=emergency */
    uint8   camera_online;          /* 1 = camera signal valid             */
    float32 radar_distance_m;       /* measured object distance in metres  */
    uint8   adas_mode;              /* 0=off 1=highway 2=city 3=parking    */
    uint8   fusion_confidence_pct;  /* sensor fusion quality 0-100         */
} Sovd_AdasData_t;

/** ECM – Engine Control Module (powertrain domain) */
typedef struct {
    uint16  engine_rpm;             /* crankshaft speed in RPM             */
    sint8   coolant_temp_c;         /* coolant temperature in °C           */
    uint8   throttle_pos_pct;       /* throttle plate opening 0-100 %      */
    uint8   fuel_level_pct;         /* tank level 0-100 %                  */
    uint8   engine_running;         /* 1 = combustion active               */
    uint8   map_kpa;                /* manifold absolute pressure in kPa   */
} Sovd_EcmData_t;

/** TCU – Transmission Control Unit (powertrain domain, simulated in HPC) */
typedef struct {
    uint8   gear_position;          /* 0=P 1=R 2=N 3=D 4-8=M1-M5          */
    uint8   trans_temp_c;           /* ATF temperature in °C               */
    uint8   shift_mode;             /* 0=automatic 1=manual 2=sport        */
    uint8   torque_converter_slip;  /* slip % 0-100                        */
    uint8   limp_home_active;       /* 1 = limp-home (fault) mode          */
} Sovd_TcuData_t;

/** BCM – Body Control Module */
typedef struct {
    uint8   door_fl_locked;         /* front-left door lock state          */
    uint8   door_fr_locked;         /* front-right door lock state         */
    uint8   door_rl_locked;         /* rear-left door lock state           */
    uint8   door_rr_locked;         /* rear-right door lock state          */
    uint8   headlights_on;          /* exterior headlight status           */
    uint8   hazard_on;              /* hazard / emergency flasher          */
    uint8   ambient_lux;            /* interior ambient light level 0-255  */
} Sovd_BcmData_t;

/** Gateway – Cross-Domain Router */
typedef struct {
    uint8   routing_active;         /* 1 = cross-domain routing enabled    */
    uint8   bus_load_pct;           /* aggregate bus utilisation 0-100 %   */
    uint8   drop_count;             /* PDUs dropped this monitoring cycle  */
    uint8   domain_count;           /* number of active domains            */
} Sovd_GwData_t;

/* =========================================================================
 * Fault store per component (mutable – can be cleared via SOVD DELETE)
 * ====================================================================== */
#define SOVD_FAULT_SLOTS  8u

typedef struct {
    Sovd_FaultRecordType faults[SOVD_FAULT_SLOTS];
    uint8                count;
} Sovd_FaultStore_t;

/* =========================================================================
 * API
 * ====================================================================== */

/** Initialise data store and mutex; populate with safe default values. */
void Sovd_DataStore_Init(void);

/** Destroy mutex; call before process exit. */
void Sovd_DataStore_Deinit(void);

/**
 * Advance simulated data by one tick (call every 100 ms from main loop).
 * Generates realistic ECU signal patterns so the dashboard is animated
 * even without live SOME/IP traffic.
 */
void Sovd_DataStore_Tick(void);

/* --- Thread-safe component getters --------------------------------------- */
void Sovd_DataStore_GetAdas   (Sovd_AdasData_t *out);
void Sovd_DataStore_GetEcm    (Sovd_EcmData_t  *out);
void Sovd_DataStore_GetTcu    (Sovd_TcuData_t  *out);
void Sovd_DataStore_GetBcm    (Sovd_BcmData_t  *out);
void Sovd_DataStore_GetGw     (Sovd_GwData_t   *out);

/* --- Thread-safe component setters (SOVD PUT writes) -------------------- */
void Sovd_DataStore_SetAdas   (const Sovd_AdasData_t *in);
void Sovd_DataStore_SetEcm    (const Sovd_EcmData_t  *in);
void Sovd_DataStore_SetTcu    (const Sovd_TcuData_t  *in);
void Sovd_DataStore_SetBcm    (const Sovd_BcmData_t  *in);
void Sovd_DataStore_SetGw     (const Sovd_GwData_t   *in);

/* --- Partial-field writers for single-element PUT ----------------------- */
Sovd_StatusType Sovd_DataStore_WriteElement(
    const char *comp, const char *element, double value);

/* --- Fault management --------------------------------------------------- */
void Sovd_DataStore_GetFaults (const char *comp, Sovd_FaultStore_t *out);
void Sovd_DataStore_ClearFaults(const char *comp);

#endif /* SOVD_DATASTORE_H */
