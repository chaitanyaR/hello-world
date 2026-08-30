/**
 * Port_17_GtmCcu6.h – Aurix TC34xx Port MCAL Driver
 * AUTOSAR Release R22-11 (SWS_Port)
 *
 * The Port driver configures the pin multiplexing and pad characteristics
 * for all device I/O pins.  For Ethernet on TC34xx this covers the RMII /
 * RGMII interface pins used to connect the EQOS MAC to the TJA1100 PHY.
 *
 * TC34xx RMII pin assignment (100BASE-T1 via TJA1100):
 *
 *   Signal       TC34xx Port.Pin  Direction  Alt-Function
 *   ----------   ---------------  ---------  ----------------
 *   ETH_REFCLK   P11.2            Input      ALT5 (ETH_CRS_DV)
 *   ETH_CRS_DV   P11.3            Input      ALT5
 *   ETH_RXD0     P11.4            Input      ALT5
 *   ETH_RXD1     P11.5            Input      ALT5
 *   ETH_TXD0     P11.6            Output     ALT6
 *   ETH_TXD1     P11.7            Output     ALT6
 *   ETH_TXEN     P11.8            Output     ALT6
 *   MDC          P12.0            Output     ALT6
 *   MDIO         P12.1            I/O        ALT6
 *
 * Reference: Aurix TC38x User Manual v1.0, Chapter 20 – Port
 *            AUTOSAR SWS_Port R22-11
 */

#ifndef PORT_17_GTMCCU6_H
#define PORT_17_GTMCCU6_H

#include "Std_Types.h"
#include "Mcal.h"

/* =========================================================================
 * Version information
 * ====================================================================== */

#define PORT_17_GTMCCU6_VENDOR_ID         (17u)
#define PORT_17_GTMCCU6_MODULE_ID         (124u)
#define PORT_17_GTMCCU6_SW_MAJOR_VERSION  (1u)
#define PORT_17_GTMCCU6_SW_MINOR_VERSION  (0u)
#define PORT_17_GTMCCU6_SW_PATCH_VERSION  (0u)

/* =========================================================================
 * Pin identifier encoding for TC34xx
 *   PORT_17_PIN(port, pin) encodes port number [7:4] and pin index [3:0]
 * ====================================================================== */

#define PORT_17_PIN(port, pin)            ((uint8)(((uint8)(port) << 4u) | (uint8)(pin)))

/* ETH0 RMII pins (see table above) */
#define PORT_17_ETH0_REFCLK               PORT_17_PIN(11u, 2u)
#define PORT_17_ETH0_CRS_DV               PORT_17_PIN(11u, 3u)
#define PORT_17_ETH0_RXD0                 PORT_17_PIN(11u, 4u)
#define PORT_17_ETH0_RXD1                 PORT_17_PIN(11u, 5u)
#define PORT_17_ETH0_TXD0                 PORT_17_PIN(11u, 6u)
#define PORT_17_ETH0_TXD1                 PORT_17_PIN(11u, 7u)
#define PORT_17_ETH0_TXEN                 PORT_17_PIN(11u, 8u)
#define PORT_17_ETH0_MDC                  PORT_17_PIN(12u, 0u)
#define PORT_17_ETH0_MDIO                 PORT_17_PIN(12u, 1u)

/* =========================================================================
 * Port configuration types (AUTOSAR SWS_Port)
 * ====================================================================== */

/** Pin direction */
typedef enum
{
    PORT_PIN_IN  = 0u,
    PORT_PIN_OUT = 1u
} Port_PinDirectionType;

/** Pad characteristics */
typedef enum
{
    PORT_PAD_STRONG_DRIVER       = 0u,
    PORT_PAD_MEDIUM_DRIVER       = 1u,
    PORT_PAD_WEAK_DRIVER         = 2u,
    PORT_PAD_TRISTATE            = 3u,
    PORT_PAD_PULL_UP             = 4u,
    PORT_PAD_PULL_DOWN           = 5u,
    PORT_PAD_OPEN_DRAIN          = 6u
} Port_PinPadType;

/** Single-pin configuration entry */
typedef struct
{
    uint8                PinId;         /* PORT_17_PIN(port, pin)    */
    Port_PinDirectionType Direction;    /* Input or Output           */
    uint8                AltFunction;   /* Alternate function select */
    Port_PinPadType      PadType;       /* Driver strength / pull    */
} Port_17_PinConfigType;

/** Module configuration (array of pin configs, NULL-terminated) */
typedef struct
{
    const Port_17_PinConfigType *PinConfigPtr;
    uint16                       PinCount;
} Port_17_GtmCcu6_ConfigType;

/* =========================================================================
 * Pre-defined configuration for ETH0 RMII pins
 * Declared here; defined in Port_17_GtmCcu6.c (or Eth_17_GEthMac.c stub).
 * ====================================================================== */

extern const Port_17_GtmCcu6_ConfigType Port_17_GtmCcu6_EthConfig;

/* =========================================================================
 * API declarations (AUTOSAR SWS_Port)
 * ====================================================================== */

/**
 * Port_17_GtmCcu6_Init               [SWS_Port_00140]
 * Configure all pins described by CfgPtr.
 */
FUNC(void, PORT_CODE)
Port_17_GtmCcu6_Init(
    P2CONST(Port_17_GtmCcu6_ConfigType, AUTOMATIC, PORT_APPL_CONST) CfgPtr);

/**
 * Port_17_GtmCcu6_SetPinDirection    [SWS_Port_00141]
 * Change direction of a pin at runtime (if allowed by pin config).
 */
FUNC(void, PORT_CODE)
Port_17_GtmCcu6_SetPinDirection(
    VAR(uint8,                  AUTOMATIC) PinId,
    VAR(Port_PinDirectionType,  AUTOMATIC) Direction);

/**
 * Port_17_GtmCcu6_RefreshPortDirection  [SWS_Port_00142]
 * Re-apply direction settings for pins not configured as changeable.
 */
FUNC(void, PORT_CODE)
Port_17_GtmCcu6_RefreshPortDirection(void);

#endif  /* PORT_17_GTMCCU6_H */
