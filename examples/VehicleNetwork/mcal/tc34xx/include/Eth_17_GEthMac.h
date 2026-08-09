/**
 * Eth_17_GEthMac.h – Aurix TC34xx Gigabit Ethernet MAC MCAL Driver
 * AUTOSAR Release R22-11 (SWS_Eth)
 *
 * MCAL Ethernet Driver for Infineon Aurix TC34xx using the on-chip
 * Synopsys EQOS (Ethernet Quality of Service) MAC controller.
 *
 * Hardware path (TC34xx production silicon):
 *   Upper BSW (EthIf)
 *       │
 *   Eth_17_GEthMac  ──► EQOS DMA descriptor rings
 *                         │
 *                     RGMII / RMII bus (P11/P13 port pins)
 *                         │
 *                     TJA1100 PHY (100BASE-T1 single-pair)
 *                         │
 *                     100BASE-T1 cable
 *
 * Host-simulation path (ETH_17_GETH_MAC_HOST_SIM):
 *   Upper BSW (EthIf / SOME/IP-SD)
 *       │
 *   Eth_17_GEthMac  ──► NodeTransport (UDP loopback socket)
 *                         │
 *                     Virtual Ethernet – Windows/Linux loopback
 *
 * Reference: AUTOSAR SWS_Eth R22-11, Sections 7–9
 *            Aurix TC38x User Manual v1.0, Chapter 23
 */

#ifndef ETH_17_GETH_MAC_H
#define ETH_17_GETH_MAC_H

#include "Eth_17_GEthMac_Cfg.h"
#include "ComStack_Types.h"

/* =========================================================================
 * Version information
 * ====================================================================== */

#define ETH_17_GETH_MAC_VENDOR_ID           (17u)   /* Infineon Technologies AG */
#define ETH_17_GETH_MAC_MODULE_ID           (88u)   /* AUTOSAR Eth module ID    */
#define ETH_17_GETH_MAC_SW_MAJOR_VERSION    (1u)
#define ETH_17_GETH_MAC_SW_MINOR_VERSION    (0u)
#define ETH_17_GETH_MAC_SW_PATCH_VERSION    (0u)

/* =========================================================================
 * Type definitions (AUTOSAR SWS_Eth)
 * ====================================================================== */

/** Ethernet controller operating mode */
typedef enum
{
    ETH_MODE_DOWN   = 0u,   /* Controller inactive (clocks may be gated) */
    ETH_MODE_ACTIVE = 1u    /* Controller active – TX/RX enabled          */
} Eth_ModeType;

/** EtherType field value in Ethernet II frame header */
typedef uint16 Eth_FrameType;

/** DMA TX buffer index (returned by ProvideTxBuffer, consumed by Transmit) */
typedef uint8  Eth_BufIdxType;

/** RX poll result */
typedef enum
{
    ETH_RECEIVED                    = 0u,  /* Valid frame received           */
    ETH_NOT_RECEIVED                = 1u,  /* No frame in RX ring            */
    ETH_RECEIVED_MORE_DATA_AVAILABLE = 2u  /* More frames pending in ring    */
} Eth_RxStatusType;

/** Controller configuration (ARXML-generated; one entry per ETH controller) */
struct Eth_17_GEthMac_ConfigType_s
{
    uint8   CtrlIdx;          /* Controller index (0 for ETH0)              */
    uint32  MacAddressHigh;   /* Bytes [5:4] of MAC in big-endian           */
    uint32  MacAddressLow;    /* Bytes [3:0] of MAC in big-endian           */
    uint8   PhyAddress;       /* MDIO address of attached PHY (0x00..0x1F)  */
    uint8   MdioCrValue;      /* MDC clock divider code (MAC_MDIO_ADDR.CR)  */
    uint8   TxDescCount;      /* Number of TX DMA descriptors               */
    uint8   RxDescCount;      /* Number of RX DMA descriptors               */
    uint16  RxBufLenByte;     /* RX buffer byte size per descriptor         */
};

/* =========================================================================
 * API declarations (AUTOSAR SWS_Eth service functions)
 * ====================================================================== */

/**
 * Eth_17_GEthMac_Init            [SWS_Eth_00027]
 * Initialise controller hardware, reset EQOS DMA, configure MAC address,
 * allocate TX/RX descriptor rings.  CfgPtr must not be NULL.
 */
FUNC(void, ETH_CODE)
Eth_17_GEthMac_Init(
    P2CONST(Eth_17_GEthMac_ConfigType, AUTOMATIC, ETH_APPL_CONST) CfgPtr);

/**
 * Eth_17_GEthMac_SetControllerMode   [SWS_Eth_00028]
 * Switch between ETH_MODE_DOWN (TX/RX stopped) and ETH_MODE_ACTIVE.
 */
FUNC(Std_ReturnType, ETH_CODE)
Eth_17_GEthMac_SetControllerMode(
    VAR(uint8,        AUTOMATIC) CtrlIdx,
    VAR(Eth_ModeType, AUTOMATIC) CtrlMode);

/**
 * Eth_17_GEthMac_GetControllerMode   [SWS_Eth_00029]
 * Returns current mode via CtrlModePtr.
 */
FUNC(Std_ReturnType, ETH_CODE)
Eth_17_GEthMac_GetControllerMode(
    VAR(uint8, AUTOMATIC)                                         CtrlIdx,
    P2VAR(Eth_ModeType, AUTOMATIC, ETH_APPL_DATA)                CtrlModePtr);

/**
 * Eth_17_GEthMac_ProvideTxBuffer     [SWS_Eth_00087]
 * Reserves a free TX DMA descriptor; returns its index and a pointer to
 * the payload buffer.  The caller fills the buffer then calls Transmit().
 */
FUNC(BufReq_ReturnType, ETH_CODE)
Eth_17_GEthMac_ProvideTxBuffer(
    VAR(uint8, AUTOMATIC)                              CtrlIdx,
    P2VAR(Eth_BufIdxType, AUTOMATIC, ETH_APPL_DATA)   BufIdxPtr,
    P2VAR(uint8 *, AUTOMATIC, ETH_APPL_DATA)           BufPtr,
    P2VAR(uint16, AUTOMATIC, ETH_APPL_DATA)            LenBytePtr);

/**
 * Eth_17_GEthMac_Transmit            [SWS_Eth_00088]
 * Hands the buffer (identified by BufIdx) to the TX DMA engine.
 * PhysAddrPtr points to the 6-byte destination MAC address.
 */
FUNC(Std_ReturnType, ETH_CODE)
Eth_17_GEthMac_Transmit(
    VAR(uint8,          AUTOMATIC) CtrlIdx,
    VAR(Eth_BufIdxType, AUTOMATIC) BufIdx,
    VAR(Eth_FrameType,  AUTOMATIC) FrameType,
    VAR(boolean,        AUTOMATIC) TxConfirmation,
    VAR(uint16,         AUTOMATIC) LenByte,
    P2CONST(uint8, AUTOMATIC, ETH_APPL_DATA) PhysAddrPtr);

/**
 * Eth_17_GEthMac_Receive             [SWS_Eth_00089]
 * Polls the RX DMA ring; for each received frame calls EthIf_RxIndication()
 * to push frames up the BSW stack.  Called from a cyclic task or ISR.
 */
FUNC(void, ETH_CODE)
Eth_17_GEthMac_Receive(
    VAR(uint8, AUTOMATIC)                                         CtrlIdx,
    P2VAR(Eth_RxStatusType, AUTOMATIC, ETH_APPL_DATA)            RxStatusPtr);

/**
 * Eth_17_GEthMac_TxConfirmation      [SWS_Eth_00090]
 * Scans TX DMA ring for completed descriptors; calls EthIf_TxConfirmation()
 * for each buffer that requested confirmation (TxConfirmation == TRUE).
 */
FUNC(void, ETH_CODE)
Eth_17_GEthMac_TxConfirmation(VAR(uint8, AUTOMATIC) CtrlIdx);

/**
 * Eth_17_GEthMac_GetVersionInfo      [SWS_Eth_00091]
 */
FUNC(void, ETH_CODE)
Eth_17_GEthMac_GetVersionInfo(
    P2VAR(Std_VersionInfoType, AUTOMATIC, ETH_APPL_DATA) VersionInfoPtr);

/* =========================================================================
 * Extended / diagnostic API (Infineon-specific)
 * ====================================================================== */

/**
 * Eth_17_GEthMac_GetPhyLinkState
 * Reads TJA1100 Basic Status via MDIO; returns TRUE if link is up.
 */
FUNC(boolean, ETH_CODE)
Eth_17_GEthMac_GetPhyLinkState(VAR(uint8, AUTOMATIC) CtrlIdx);

/**
 * Eth_17_GEthMac_ReadMii / WriteMii
 * Direct MDIO register access for PHY initialisation and diagnostics.
 */
FUNC(Std_ReturnType, ETH_CODE)
Eth_17_GEthMac_ReadMii(
    VAR(uint8, AUTOMATIC) CtrlIdx,
    VAR(uint8, AUTOMATIC) PhyAddr,
    VAR(uint8, AUTOMATIC) RegAddr,
    P2VAR(uint16, AUTOMATIC, ETH_APPL_DATA) RegDataPtr);

FUNC(Std_ReturnType, ETH_CODE)
Eth_17_GEthMac_WriteMii(
    VAR(uint8,  AUTOMATIC) CtrlIdx,
    VAR(uint8,  AUTOMATIC) PhyAddr,
    VAR(uint8,  AUTOMATIC) RegAddr,
    VAR(uint16, AUTOMATIC) RegData);

#endif  /* ETH_17_GETH_MAC_H */
