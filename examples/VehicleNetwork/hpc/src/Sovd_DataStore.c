/**
 * Sovd_DataStore.c  –  Thread-Safe Vehicle Data Store Implementation
 *
 * Provides simulated ECU data that advances each tick (100 ms) and can be
 * overridden by real SOME/IP callbacks or SOVD PUT requests.  All public
 * functions acquire/release the internal mutex to allow concurrent access
 * from the SOVD HTTP server thread and the SOME/IP receive thread.
 */

#include "Sovd_DataStore.h"
#include "Sovd_Types.h"

#include <string.h>
#include <stdio.h>
#include <math.h>    /* sinf, cosf – link with -lm on Linux */

/* =========================================================================
 * Platform mutex helpers
 * ====================================================================== */
#ifdef _WIN32
#  define DS_MUTEX_INIT(m)    InitializeCriticalSection(&(m))
#  define DS_MUTEX_LOCK(m)    EnterCriticalSection(&(m))
#  define DS_MUTEX_UNLOCK(m)  LeaveCriticalSection(&(m))
#  define DS_MUTEX_DESTROY(m) DeleteCriticalSection(&(m))
#else
#  define DS_MUTEX_INIT(m)    pthread_mutex_init(&(m), NULL)
#  define DS_MUTEX_LOCK(m)    pthread_mutex_lock(&(m))
#  define DS_MUTEX_UNLOCK(m)  pthread_mutex_unlock(&(m))
#  define DS_MUTEX_DESTROY(m) pthread_mutex_destroy(&(m))
#endif

/* =========================================================================
 * Static store
 * ====================================================================== */
static struct {
    Sovd_AdasData_t  adas;
    Sovd_EcmData_t   ecm;
    Sovd_TcuData_t   tcu;
    Sovd_BcmData_t   bcm;
    Sovd_GwData_t    gw;

    Sovd_FaultStore_t faults_adas;
    Sovd_FaultStore_t faults_ecm;
    Sovd_FaultStore_t faults_tcu;
    Sovd_FaultStore_t faults_bcm;
    Sovd_FaultStore_t faults_gw;

    Sovd_MutexType lock;
    uint32         tick;
} g_ds;

/* =========================================================================
 * Pre-defined fault catalogue (ROM constant strings)
 * ====================================================================== */
static const Sovd_FaultRecordType k_faults_adas[] = {
    { 0x001001u, SOVD_DTC_STATUS_CONFIRMED, SOVD_DTC_SEV_CHECK_AT_NEXT_HALT,
      1u, "P1001 – Front Camera Signal Lost" },
    { 0x001002u, SOVD_DTC_STATUS_PENDING,   SOVD_DTC_SEV_MAINTENANCE_ONLY,
      1u, "P1002 – Radar Module Response Timeout" },
};

static const Sovd_FaultRecordType k_faults_ecm[] = {
    { 0x000300u, SOVD_DTC_STATUS_CONFIRMED, SOVD_DTC_SEV_CHECK_IMMEDIATELY,
      1u, "P0300 – Random/Multiple Cylinder Misfire Detected" },
    { 0x000128u, SOVD_DTC_STATUS_PENDING,   SOVD_DTC_SEV_MAINTENANCE_ONLY,
      1u, "P0128 – Coolant Thermostat Below Regulating Temperature" },
};

static const Sovd_FaultRecordType k_faults_tcu[] = {
    { 0x000700u, SOVD_DTC_STATUS_CONFIRMED, SOVD_DTC_SEV_CHECK_AT_NEXT_HALT,
      1u, "P0700 – Transmission Control System Malfunction" },
    { 0x000720u, SOVD_DTC_STATUS_PENDING,   SOVD_DTC_SEV_MAINTENANCE_ONLY,
      1u, "P0720 – Output Speed Sensor Circuit No Signal" },
};

static const Sovd_FaultRecordType k_faults_bcm[] = {
    { 0x00B001u, SOVD_DTC_STATUS_PENDING,   SOVD_DTC_SEV_MAINTENANCE_ONLY,
      1u, "B1001 – Front-Left Door Ajar Switch Stuck" },
    { 0x00B002u, SOVD_DTC_STATUS_CONFIRMED, SOVD_DTC_SEV_CHECK_AT_NEXT_HALT,
      1u, "B1002 – Front-Left Power Window Motor Fault" },
};

static const Sovd_FaultRecordType k_faults_gw[] = {
    { 0x00C100u, SOVD_DTC_STATUS_PENDING,   SOVD_DTC_SEV_MAINTENANCE_ONLY,
      1u, "U0100 – Lost Communication With ECM/PCM (CAN timeout)" },
};

static void load_fault_store(Sovd_FaultStore_t *store,
                              const Sovd_FaultRecordType *src, uint8 count)
{
    uint8 i;
    if (count > SOVD_FAULT_SLOTS) count = SOVD_FAULT_SLOTS;
    for (i = 0u; i < count; i++) {
        store->faults[i] = src[i];
    }
    store->count = count;
}

/* =========================================================================
 * Public: Init / Deinit
 * ====================================================================== */
void Sovd_DataStore_Init(void)
{
    (void)memset(&g_ds, 0, sizeof(g_ds));
    DS_MUTEX_INIT(g_ds.lock);

    /* --- ADAS defaults -------------------------------------------------- */
    g_ds.adas.camera_online         = TRUE;
    g_ds.adas.adas_mode             = 1u;    /* highway */
    g_ds.adas.radar_distance_m      = 30.0f;
    g_ds.adas.fusion_confidence_pct = 95u;

    /* --- ECM defaults --------------------------------------------------- */
    g_ds.ecm.engine_running  = TRUE;
    g_ds.ecm.engine_rpm      = 1800u;
    g_ds.ecm.coolant_temp_c  = 20;    /* cold start */
    g_ds.ecm.throttle_pos_pct = 15u;
    g_ds.ecm.fuel_level_pct  = 75u;
    g_ds.ecm.map_kpa         = 101u;

    /* --- TCU defaults --------------------------------------------------- */
    g_ds.tcu.gear_position        = 3u;   /* Drive */
    g_ds.tcu.trans_temp_c         = 60u;
    g_ds.tcu.shift_mode           = 0u;   /* automatic */
    g_ds.tcu.torque_converter_slip = 2u;

    /* --- BCM defaults --------------------------------------------------- */
    g_ds.bcm.door_fl_locked = TRUE;
    g_ds.bcm.door_fr_locked = TRUE;
    g_ds.bcm.door_rl_locked = TRUE;
    g_ds.bcm.door_rr_locked = TRUE;
    g_ds.bcm.headlights_on  = FALSE;
    g_ds.bcm.ambient_lux    = 128u;

    /* --- Gateway defaults ----------------------------------------------- */
    g_ds.gw.routing_active = TRUE;
    g_ds.gw.bus_load_pct   = 35u;
    g_ds.gw.domain_count   = 3u;

    /* --- Fault catalogue ----------------------------------------------- */
    load_fault_store(&g_ds.faults_adas,
                     k_faults_adas,
                     (uint8)(sizeof(k_faults_adas)/sizeof(k_faults_adas[0])));
    load_fault_store(&g_ds.faults_ecm,
                     k_faults_ecm,
                     (uint8)(sizeof(k_faults_ecm)/sizeof(k_faults_ecm[0])));
    load_fault_store(&g_ds.faults_tcu,
                     k_faults_tcu,
                     (uint8)(sizeof(k_faults_tcu)/sizeof(k_faults_tcu[0])));
    load_fault_store(&g_ds.faults_bcm,
                     k_faults_bcm,
                     (uint8)(sizeof(k_faults_bcm)/sizeof(k_faults_bcm[0])));
    load_fault_store(&g_ds.faults_gw,
                     k_faults_gw,
                     (uint8)(sizeof(k_faults_gw)/sizeof(k_faults_gw[0])));
}

void Sovd_DataStore_Deinit(void)
{
    DS_MUTEX_DESTROY(g_ds.lock);
}

/* =========================================================================
 * Public: Tick – advance simulated vehicle signals every 100 ms
 * ====================================================================== */
void Sovd_DataStore_Tick(void)
{
    float t;

    DS_MUTEX_LOCK(g_ds.lock);

    g_ds.tick++;
    t = (float)g_ds.tick * 0.1f;  /* time in seconds */

    /* ECM: engine RPM oscillates around cruise point */
    g_ds.ecm.engine_rpm =
        (uint16)(1800.0f + 700.0f * sinf(t * 0.31f) + 200.0f * sinf(t * 1.1f));

    /* ECM: coolant warms from cold start (20 °C) to 90 °C in ~2 min */
    if (g_ds.ecm.coolant_temp_c < 90) {
        g_ds.ecm.coolant_temp_c = (sint8)(g_ds.ecm.coolant_temp_c + 1);
    } else {
        g_ds.ecm.coolant_temp_c = (sint8)(88 + (int)(4.0f * sinf(t * 0.17f)));
    }

    /* ECM: MAP tracks throttle and RPM */
    g_ds.ecm.map_kpa = (uint8)(60u + (uint8)(40.0f * (sinf(t * 0.29f) + 1.0f) / 2.0f));

    /* ECM: fuel drains slowly – every 200 ticks (20 s) drop 1% */
    if (((g_ds.tick % 200u) == 0u) && (g_ds.ecm.fuel_level_pct > 5u)) {
        g_ds.ecm.fuel_level_pct--;
    }

    /* ADAS: radar distance – approaching/receding target simulation */
    g_ds.adas.radar_distance_m =
        20.0f + 18.0f * sinf(t * 0.43f) + 5.0f * sinf(t * 1.3f);
    if (g_ds.adas.radar_distance_m < 0.0f) g_ds.adas.radar_distance_m = 0.5f;

    /* ADAS: collision warning derived from radar distance */
    if (g_ds.adas.radar_distance_m < 6.0f) {
        g_ds.adas.collision_warn_level = 3u;  /* EMERGENCY */
    } else if (g_ds.adas.radar_distance_m < 12.0f) {
        g_ds.adas.collision_warn_level = 2u;  /* WARNING   */
    } else if (g_ds.adas.radar_distance_m < 20.0f) {
        g_ds.adas.collision_warn_level = 1u;  /* CAUTION   */
    } else {
        g_ds.adas.collision_warn_level = 0u;  /* CLEAR     */
    }

    /* ADAS: lane departure – brief event every ~300 ticks */
    g_ds.adas.lane_departure_active =
        ((g_ds.tick % 300u) > 285u) ? TRUE : FALSE;

    /* ADAS: fusion confidence varies slightly */
    g_ds.adas.fusion_confidence_pct =
        (uint8)(92u + (uint8)(6.0f * (sinf(t * 0.07f) + 1.0f) / 2.0f));

    /* TCU: auto gear selection based on RPM */
    if ((g_ds.tcu.shift_mode == 0u) && g_ds.ecm.engine_running) {
        if ((g_ds.ecm.engine_rpm > 3200u) && (g_ds.tcu.gear_position < 6u)) {
            g_ds.tcu.gear_position++;
        } else if ((g_ds.ecm.engine_rpm < 1100u) && (g_ds.tcu.gear_position > 3u)) {
            g_ds.tcu.gear_position--;
        }
    }

    /* TCU: trans temp rises with RPM, cools slowly */
    {
        uint8 target = (uint8)(65u + (g_ds.ecm.engine_rpm / 100u));
        if (g_ds.tcu.trans_temp_c < target) g_ds.tcu.trans_temp_c++;
        else if (g_ds.tcu.trans_temp_c > target + 2u) g_ds.tcu.trans_temp_c--;
    }

    /* TCU: torque converter slip tracks RPM delta */
    g_ds.tcu.torque_converter_slip =
        (uint8)(2u + (uint8)(5.0f * (sinf(t * 0.8f) + 1.0f) / 2.0f));

    /* BCM: ambient light cycles through a day (very compressed) */
    g_ds.bcm.ambient_lux =
        (uint8)(128u + (uint8)(120.0f * sinf(t * 0.05f)));

    /* Gateway: bus load scales with message activity */
    g_ds.gw.bus_load_pct =
        (uint8)(35u + (uint8)(35.0f * (sinf(t * 0.23f) + 1.0f) / 2.0f));
    g_ds.gw.drop_count =
        (g_ds.gw.bus_load_pct > 80u) ? (uint8)((g_ds.tick % 5u) + 1u) : 0u;

    DS_MUTEX_UNLOCK(g_ds.lock);
}

/* =========================================================================
 * Thread-safe getters
 * ====================================================================== */
void Sovd_DataStore_GetAdas(Sovd_AdasData_t *out) {
    DS_MUTEX_LOCK(g_ds.lock);
    *out = g_ds.adas;
    DS_MUTEX_UNLOCK(g_ds.lock);
}
void Sovd_DataStore_GetEcm(Sovd_EcmData_t *out) {
    DS_MUTEX_LOCK(g_ds.lock);
    *out = g_ds.ecm;
    DS_MUTEX_UNLOCK(g_ds.lock);
}
void Sovd_DataStore_GetTcu(Sovd_TcuData_t *out) {
    DS_MUTEX_LOCK(g_ds.lock);
    *out = g_ds.tcu;
    DS_MUTEX_UNLOCK(g_ds.lock);
}
void Sovd_DataStore_GetBcm(Sovd_BcmData_t *out) {
    DS_MUTEX_LOCK(g_ds.lock);
    *out = g_ds.bcm;
    DS_MUTEX_UNLOCK(g_ds.lock);
}
void Sovd_DataStore_GetGw(Sovd_GwData_t *out) {
    DS_MUTEX_LOCK(g_ds.lock);
    *out = g_ds.gw;
    DS_MUTEX_UNLOCK(g_ds.lock);
}

/* =========================================================================
 * Thread-safe setters (bulk – used by SOME/IP RX callbacks)
 * ====================================================================== */
void Sovd_DataStore_SetAdas(const Sovd_AdasData_t *in) {
    DS_MUTEX_LOCK(g_ds.lock);
    g_ds.adas = *in;
    DS_MUTEX_UNLOCK(g_ds.lock);
}
void Sovd_DataStore_SetEcm(const Sovd_EcmData_t *in) {
    DS_MUTEX_LOCK(g_ds.lock);
    g_ds.ecm = *in;
    DS_MUTEX_UNLOCK(g_ds.lock);
}
void Sovd_DataStore_SetTcu(const Sovd_TcuData_t *in) {
    DS_MUTEX_LOCK(g_ds.lock);
    g_ds.tcu = *in;
    DS_MUTEX_UNLOCK(g_ds.lock);
}
void Sovd_DataStore_SetBcm(const Sovd_BcmData_t *in) {
    DS_MUTEX_LOCK(g_ds.lock);
    g_ds.bcm = *in;
    DS_MUTEX_UNLOCK(g_ds.lock);
}
void Sovd_DataStore_SetGw(const Sovd_GwData_t *in) {
    DS_MUTEX_LOCK(g_ds.lock);
    g_ds.gw = *in;
    DS_MUTEX_UNLOCK(g_ds.lock);
}

/* =========================================================================
 * Write single element by name (SOVD PUT handler)
 * ====================================================================== */
Sovd_StatusType Sovd_DataStore_WriteElement(
    const char *comp, const char *element, double value)
{
    Sovd_StatusType rc = SOVD_STATUS_NOT_FOUND;
    uint8 uv = (uint8)(value < 0.0 ? 0u : (value > 255.0 ? 255u : (uint8)value));
    int   iv = (int)value;

    DS_MUTEX_LOCK(g_ds.lock);

    if (strcmp(comp, SOVD_COMP_ADAS) == 0) {
        if (strcmp(element, "adas_mode") == 0) {
            g_ds.adas.adas_mode = (uint8)(uv & 0x03u);
            rc = SOVD_STATUS_OK;
        }
    } else if (strcmp(comp, SOVD_COMP_ECM) == 0) {
        if (strcmp(element, "throttle_pos_pct") == 0) {
            g_ds.ecm.throttle_pos_pct = (uint8)(uv > 100u ? 100u : uv);
            rc = SOVD_STATUS_OK;
        }
    } else if (strcmp(comp, SOVD_COMP_TCU) == 0) {
        if (strcmp(element, "gear_position") == 0) {
            g_ds.tcu.gear_position = (uint8)(uv > 8u ? 8u : uv);
            rc = SOVD_STATUS_OK;
        } else if (strcmp(element, "shift_mode") == 0) {
            g_ds.tcu.shift_mode = (uint8)(uv > 2u ? 2u : uv);
            rc = SOVD_STATUS_OK;
        }
    } else if (strcmp(comp, SOVD_COMP_BCM) == 0) {
        uint8 bv = (iv != 0) ? TRUE : FALSE;
        if (strcmp(element, "door_fl_locked") == 0) {
            g_ds.bcm.door_fl_locked = bv; rc = SOVD_STATUS_OK;
        } else if (strcmp(element, "door_fr_locked") == 0) {
            g_ds.bcm.door_fr_locked = bv; rc = SOVD_STATUS_OK;
        } else if (strcmp(element, "door_rl_locked") == 0) {
            g_ds.bcm.door_rl_locked = bv; rc = SOVD_STATUS_OK;
        } else if (strcmp(element, "door_rr_locked") == 0) {
            g_ds.bcm.door_rr_locked = bv; rc = SOVD_STATUS_OK;
        } else if (strcmp(element, "headlights_on") == 0) {
            g_ds.bcm.headlights_on = bv; rc = SOVD_STATUS_OK;
        } else if (strcmp(element, "hazard_on") == 0) {
            g_ds.bcm.hazard_on = bv; rc = SOVD_STATUS_OK;
        }
    }

    DS_MUTEX_UNLOCK(g_ds.lock);
    return rc;
}

/* =========================================================================
 * Fault management
 * ====================================================================== */
static Sovd_FaultStore_t *fault_store_for(const char *comp)
{
    if (strcmp(comp, SOVD_COMP_ADAS)    == 0) return &g_ds.faults_adas;
    if (strcmp(comp, SOVD_COMP_ECM)     == 0) return &g_ds.faults_ecm;
    if (strcmp(comp, SOVD_COMP_TCU)     == 0) return &g_ds.faults_tcu;
    if (strcmp(comp, SOVD_COMP_BCM)     == 0) return &g_ds.faults_bcm;
    if (strcmp(comp, SOVD_COMP_GATEWAY) == 0) return &g_ds.faults_gw;
    return NULL;
}

void Sovd_DataStore_GetFaults(const char *comp, Sovd_FaultStore_t *out)
{
    DS_MUTEX_LOCK(g_ds.lock);
    {
        const Sovd_FaultStore_t *src = fault_store_for(comp);
        if (src) *out = *src;
        else     (void)memset(out, 0, sizeof(*out));
    }
    DS_MUTEX_UNLOCK(g_ds.lock);
}

void Sovd_DataStore_ClearFaults(const char *comp)
{
    DS_MUTEX_LOCK(g_ds.lock);
    {
        Sovd_FaultStore_t *store = fault_store_for(comp);
        if (store) {
            uint8 i;
            for (i = 0u; i < store->count; i++) {
                store->faults[i].active     = FALSE;
                store->faults[i].statusByte = 0u;
            }
        }
    }
    DS_MUTEX_UNLOCK(g_ds.lock);
}
