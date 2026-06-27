/**
 * EthIf.h – Ethernet Interface Module
 * AUTOSAR Release R22-11 (SWS_EthIf)
 *
 * EthIf sits between the upper communication stack (SoAd / SOME/IP) and
 * the lower MCAL Ethernet driver (Eth_17_GEthMac).  It provides a uniform
 * interface across multiple Ethernet controllers and handles:
 *   - Controller lifecycle management (init, mode control)
 *   - Frame TX path: EthIf_Transmit() → Eth_17_GEthMac_Transmit()
 *   - Frame RX path: Eth_17_GEthMac_Receive() → EthIf_RxIndication() → SoAd
 *   - TX confirmation: Eth_17_GEthMac_TxConfirmation() → EthIf_TxConfirmation()
 *
 * In the vehicle-network simulation, EthIf delegates directly to
 * Eth_17_GEthMac which in HOST_SIM mode calls NodeTransport.
 */

#ifndef ETHIF_H
#define ETHIF_H

#include "Compiler.h"    /* FUNC(), P2CONST(), P2VAR() macros */
#include "Std_Types.h"
#include "ComStack_Types.h"
#include "Eth_17_GEthMac.h"

/* =========================================================================
 * Version information
 * ====================================================================== */

#define ETHIF_VENDOR_ID           (17u)
#define ETHIF_MODULE_ID           (65u)
#define ETHIF_SW_MAJOR_VERSION    (1u)
#define ETHIF_SW_MINOR_VERSION    (0u)
#define ETHIF_SW_PATCH_VERSION    (0u)

/* =========================================================================
 * EtherType constants (IEEE 802.3)
 * ====================================================================== */

#define ETHIF_ETHERTYPE_IPV4          ((Eth_FrameType)0x0800u)
#define ETHIF_ETHERTYPE_IPV6          ((Eth_FrameType)0x86DDu)
#define ETHIF_ETHERTYPE_VLAN          ((Eth_FrameType)0x8100u)
#define ETHIF_ETHERTYPE_SOMEIP        ((Eth_FrameType)0x0800u)  /* SOME/IP over UDP/IP */
#define ETHIF_ETHERTYPE_AVTP          ((Eth_FrameType)0x22F0u)  /* Audio/Video Transport */

/* =========================================================================
 * EthIf configuration types
 * ====================================================================== */

/** EthIf controller configuration (one per ETH controller) */
typedef struct
{
    uint8  EthIfCtrlIdx;          /* EthIf controller index (matches Eth CtrlIdx) */
    uint8  EthCtrlIdx;            /* Lower-layer Eth MCAL controller index         */
    uint16 EthIfMaxRxFrameSize;   /* Maximum receive frame size in bytes            */
    uint16 EthIfMaxTxFrameSize;   /* Maximum transmit frame size in bytes           */
} EthIf_ConfigType;

/* =========================================================================
 * API declarations (AUTOSAR SWS_EthIf)
 * ====================================================================== */

/**
 * EthIf_Init                              [SWS_EthIf_00030]
 * Initialises all Ethernet controllers via Eth_17_GEthMac_Init().
 */
FUNC(void, ETHIF_CODE)
EthIf_Init(P2CONST(EthIf_ConfigType, AUTOMATIC, ETHIF_APPL_CONST) CfgPtr);

/**
 * EthIf_SetControllerMode                 [SWS_EthIf_00041]
 * Forwards mode change to the lower-layer MCAL driver.
 */
FUNC(Std_ReturnType, ETHIF_CODE)
EthIf_SetControllerMode(
    VAR(uint8,        AUTOMATIC) CtrlIdx,
    VAR(Eth_ModeType, AUTOMATIC) CtrlMode);

/**
 * EthIf_GetControllerMode                 [SWS_EthIf_00042]
 */
FUNC(Std_ReturnType, ETHIF_CODE)
EthIf_GetControllerMode(
    VAR(uint8, AUTOMATIC)                                         CtrlIdx,
    P2VAR(Eth_ModeType, AUTOMATIC, ETHIF_APPL_DATA)              CtrlModePtr);

/**
 * EthIf_ProvideTxBuffer                   [SWS_EthIf_00087]
 * Requests a transmit buffer from the lower MCAL layer.
 */
FUNC(BufReq_ReturnType, ETHIF_CODE)
EthIf_ProvideTxBuffer(
    VAR(uint8, AUTOMATIC)                              CtrlIdx,
    VAR(Eth_FrameType, AUTOMATIC)                      FrameType,
    P2VAR(Eth_BufIdxType, AUTOMATIC, ETHIF_APPL_DATA) BufIdxPtr,
    P2VAR(uint8 *, AUTOMATIC, ETHIF_APPL_DATA)         BufPtr,
    P2VAR(uint16, AUTOMATIC, ETHIF_APPL_DATA)          LenBytePtr);

/**
 * EthIf_Transmit                          [SWS_EthIf_00088]
 * Forwards the filled TX buffer to the lower MCAL driver.
 */
FUNC(Std_ReturnType, ETHIF_CODE)
EthIf_Transmit(
    VAR(uint8,          AUTOMATIC) CtrlIdx,
    VAR(Eth_BufIdxType, AUTOMATIC) BufIdx,
    VAR(Eth_FrameType,  AUTOMATIC) FrameType,
    VAR(boolean,        AUTOMATIC) TxConfirmation,
    VAR(uint16,         AUTOMATIC) LenByte,
    P2CONST(uint8, AUTOMATIC, ETHIF_APPL_DATA) PhysAddrPtr);

/**
 * EthIf_RxIndication                      [SWS_EthIf_00089]
 * Called by the MCAL driver for each received frame; routes the frame to
 * the correct upper-layer handler (SoAd) via registered EtherType.
 */
FUNC(void, ETHIF_CODE)
EthIf_RxIndication(
    VAR(uint8,         AUTOMATIC) CtrlIdx,
    VAR(Eth_FrameType, AUTOMATIC) FrameType,
    VAR(boolean,       AUTOMATIC) IsBroadcast,
    P2CONST(uint8, AUTOMATIC, ETHIF_APPL_DATA) PhysAddrPtr,
    P2VAR(uint8,  AUTOMATIC, ETHIF_APPL_DATA)  DataPtr,
    VAR(uint16,        AUTOMATIC) LenByte);

/**
 * EthIf_TxConfirmation                    [SWS_EthIf_00090]
 * Called by MCAL when TX DMA has completed; notifies upper layer.
 */
FUNC(void, ETHIF_CODE)
EthIf_TxConfirmation(
    VAR(uint8,          AUTOMATIC) CtrlIdx,
    VAR(Eth_BufIdxType, AUTOMATIC) BufIdx);

/**
 * EthIf_MainFunctionRx / EthIf_MainFunctionTx    [SWS_EthIf_00098/99]
 * Called from an AUTOSAR OS cyclic task (10 ms BSW task in our setup).
 * EthIf_MainFunctionRx polls the RX ring; EthIf_MainFunctionTx polls
 * the TX ring for confirmation callbacks.
 */
FUNC(void, ETHIF_CODE) EthIf_MainFunctionRx(void);
FUNC(void, ETHIF_CODE) EthIf_MainFunctionTx(void);

/**
 * EthIf_GetVersionInfo                    [SWS_EthIf_00095]
 */
FUNC(void, ETHIF_CODE)
EthIf_GetVersionInfo(
    P2VAR(Std_VersionInfoType, AUTOMATIC, ETHIF_APPL_DATA) VersionInfoPtr);

#endif  /* ETHIF_H */
