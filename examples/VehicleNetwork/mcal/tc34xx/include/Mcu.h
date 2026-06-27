/**
 * Mcu.h – Aurix TC34xx MCU Driver
 * AUTOSAR Release R22-11 (SWS_Mcu)
 *
 * The MCU driver initialises the processor clock tree and provides
 * access to reset, power-management and core configuration services.
 *
 * TC34xx clock topology (relevant to Ethernet):
 *
 *   OSC0 (20 MHz crystal)
 *      └─► fPLL0 (200 MHz) ─► fSRI (200 MHz)  [CPU bus]
 *                           ─► fPER (100 MHz)  [peripheral bus → EQOS MDC]
 *                           ─► fSTM (100 MHz)  [System Timer]
 *                           ─► fETH (50 MHz)   [RMII reference clock to PHY]
 *
 * fETH = 50 MHz is generated from fPLL0 via the CCU6/GTM clock tree and
 * routed to the PHYCLK output pin that drives the TJA1100 PHY.
 *
 * Reference: Aurix TC3x Architecture Reference Manual v2.1, Chapter 2
 *            AUTOSAR SWS_Mcu R22-11
 */

#ifndef MCU_H
#define MCU_H

#include "Std_Types.h"
#include "Mcal.h"

/* =========================================================================
 * Version information
 * ====================================================================== */

#define MCU_VENDOR_ID           (17u)
#define MCU_MODULE_ID           (101u)
#define MCU_SW_MAJOR_VERSION    (1u)
#define MCU_SW_MINOR_VERSION    (0u)
#define MCU_SW_PATCH_VERSION    (0u)

/* =========================================================================
 * MCU types (AUTOSAR SWS_Mcu)
 * ====================================================================== */

typedef uint8   Mcu_ClockType;     /* Clock setting index (index into config)   */
typedef uint32  Mcu_RawResetType;  /* Raw reset cause register value             */
typedef uint8   Mcu_ModeType;      /* Low-power / normal mode identifier         */

typedef enum
{
    MCU_POWER_ON_RESET  = 0u,
    MCU_WATCHDOG_RESET  = 1u,
    MCU_SW_RESET        = 2u,
    MCU_RESET_UNDEFINED = 3u
} Mcu_ResetType;

typedef enum
{
    MCU_PLL_LOCKED    = 0u,
    MCU_PLL_UNLOCKED  = 1u,
    MCU_PLL_STATUS_UNDEFINED = 2u
} Mcu_PllStatusType;

/* =========================================================================
 * Clock configuration (generated from ARXML in production)
 * ====================================================================== */

/** Clock setting index for the Ethernet peripheral clock */
#define MCU_CLOCK_SETTING_ETH    ((Mcu_ClockType)2u)

/** Peripheral clock frequency in Hz (fPER = 100 MHz) */
#define MCU_FPER_HZ              (100000000uL)

/** Ethernet reference clock for TJA1100 RMII (fETH = 50 MHz) */
#define MCU_FETH_HZ              (50000000uL)

/** Module configuration type */
typedef struct
{
    uint32 PllFrequencyHz;        /* Target fPLL0 (200 MHz)           */
    uint32 PerClockDivider;       /* fPER = fPLL0 / PerClockDivider   */
    uint32 EthClockDivider;       /* fETH = fPLL0 / EthClockDivider   */
    uint32 WatchdogTimeoutMs;     /* Safety WDT timeout in ms         */
} Mcu_ConfigType;

extern const Mcu_ConfigType Mcu_Config;

/* =========================================================================
 * API declarations (AUTOSAR SWS_Mcu)
 * ====================================================================== */

/**
 * Mcu_Init                [SWS_Mcu_00153]
 * Configures the MCU from the provided configuration pointer.
 * On host simulation: no-op.
 */
FUNC(void, MCU_CODE)
Mcu_Init(P2CONST(Mcu_ConfigType, AUTOMATIC, MCU_APPL_CONST) CfgPtr);

/**
 * Mcu_InitClock           [SWS_Mcu_00155]
 * Activates the PLL and peripheral clock tree for the given ClockSetting.
 * Returns E_OK when PLL is locked, E_NOT_OK on timeout.
 */
FUNC(Std_ReturnType, MCU_CODE)
Mcu_InitClock(VAR(Mcu_ClockType, AUTOMATIC) ClockSetting);

/**
 * Mcu_GetPllStatus        [SWS_Mcu_00008]
 * Returns the current PLL lock status.
 */
FUNC(Mcu_PllStatusType, MCU_CODE)
Mcu_GetPllStatus(void);

/**
 * Mcu_DistributePllClock  [SWS_Mcu_00156]
 * Switches the system to the PLL clock after Mcu_InitClock() has confirmed
 * lock.  Prerequisite: Mcu_GetPllStatus() == MCU_PLL_LOCKED.
 */
FUNC(void, MCU_CODE)
Mcu_DistributePllClock(void);

/**
 * Mcu_GetResetReason      [SWS_Mcu_00005]
 */
FUNC(Mcu_ResetType, MCU_CODE)
Mcu_GetResetReason(void);

/**
 * Mcu_GetResetRawValue    [SWS_Mcu_00006]
 */
FUNC(Mcu_RawResetType, MCU_CODE)
Mcu_GetResetRawValue(void);

/**
 * Mcu_PerformReset        [SWS_Mcu_00055]
 */
FUNC(void, MCU_CODE)
Mcu_PerformReset(void);

/**
 * Mcu_SetMode             [SWS_Mcu_00007]
 */
FUNC(void, MCU_CODE)
Mcu_SetMode(VAR(Mcu_ModeType, AUTOMATIC) McuMode);

#endif  /* MCU_H */
