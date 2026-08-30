/**
 * EthIf.c – Ethernet Interface Module (stub)
 * AUTOSAR Release R22-11
 *
 * EthIf sits between the SOME/IP / SoAd upper layer and the Eth MCAL.
 * In the vehicle-network simulation the full EthIf routing logic is not
 * needed because SOME/IP-SD frames are injected directly via
 * SomeIpSd_Transmit() → NodeTransport without going through EthIf.
 *
 * This file provides AUTOSAR-compliant stubs so that any BSW module that
 * calls EthIf_* functions compiles and links correctly.  The main-function
 * task wrappers (EthIf_MainFunctionRx/Tx) delegate to the MCAL.
 */

#include "EthIf.h"
#include "Eth_17_GEthMac.h"
#include <stdio.h>

/* Module-static state */
static const EthIf_ConfigType *EthIf_Cfg   = NULL;   /* PRQA S 1531 */
static boolean                  EthIf_Inited = FALSE; /* PRQA S 1531 */

/* =========================================================================
 * EthIf_Init                                           [SWS_EthIf_00030]
 * ====================================================================== */

FUNC(void, ETHIF_CODE)
EthIf_Init(P2CONST(EthIf_ConfigType, AUTOMATIC, ETHIF_APPL_CONST) CfgPtr)
{
    if (CfgPtr == NULL) { return; }
    EthIf_Cfg   = CfgPtr;
    EthIf_Inited = TRUE;
    (void)EthIf_Cfg;      /* suppress unused in simulation builds */
    (void)EthIf_Inited;
    Eth_17_GEthMac_Init(&Eth_17_GEthMac_Config);
    printf("[EthIf] Initialised  (ctrl_count=%u)\n",
           (unsigned)ETH_17_GETH_MAC_CTRL_COUNT);
}

/* =========================================================================
 * EthIf_SetControllerMode                              [SWS_EthIf_00041]
 * ====================================================================== */

FUNC(Std_ReturnType, ETHIF_CODE)
EthIf_SetControllerMode(
    VAR(uint8,        AUTOMATIC) CtrlIdx,
    VAR(Eth_ModeType, AUTOMATIC) CtrlMode)
{
    return Eth_17_GEthMac_SetControllerMode(CtrlIdx, CtrlMode);
}

/* =========================================================================
 * EthIf_GetControllerMode                              [SWS_EthIf_00042]
 * ====================================================================== */

FUNC(Std_ReturnType, ETHIF_CODE)
EthIf_GetControllerMode(
    VAR(uint8, AUTOMATIC)                             CtrlIdx,
    P2VAR(Eth_ModeType, AUTOMATIC, ETHIF_APPL_DATA)   CtrlModePtr)
{
    return Eth_17_GEthMac_GetControllerMode(CtrlIdx, CtrlModePtr);
}

/* =========================================================================
 * EthIf_ProvideTxBuffer                                [SWS_EthIf_00087]
 * ====================================================================== */

FUNC(BufReq_ReturnType, ETHIF_CODE)
EthIf_ProvideTxBuffer(
    VAR(uint8, AUTOMATIC)                              CtrlIdx,
    VAR(Eth_FrameType, AUTOMATIC)                      FrameType,
    P2VAR(Eth_BufIdxType, AUTOMATIC, ETHIF_APPL_DATA)  BufIdxPtr,
    P2VAR(uint8 *, AUTOMATIC, ETHIF_APPL_DATA)          BufPtr,
    P2VAR(uint16, AUTOMATIC, ETHIF_APPL_DATA)           LenBytePtr)
{
    (void)FrameType;
    return Eth_17_GEthMac_ProvideTxBuffer(CtrlIdx, BufIdxPtr, BufPtr, LenBytePtr);
}

/* =========================================================================
 * EthIf_Transmit                                       [SWS_EthIf_00088]
 * ====================================================================== */

FUNC(Std_ReturnType, ETHIF_CODE)
EthIf_Transmit(
    VAR(uint8,          AUTOMATIC) CtrlIdx,
    VAR(Eth_BufIdxType, AUTOMATIC) BufIdx,
    VAR(Eth_FrameType,  AUTOMATIC) FrameType,
    VAR(boolean,        AUTOMATIC) TxConfirmation,
    VAR(uint16,         AUTOMATIC) LenByte,
    P2CONST(uint8, AUTOMATIC, ETHIF_APPL_DATA) PhysAddrPtr)
{
    return Eth_17_GEthMac_Transmit(CtrlIdx, BufIdx, FrameType,
                                    TxConfirmation, LenByte, PhysAddrPtr);
}

/* =========================================================================
 * EthIf_RxIndication                                   [SWS_EthIf_00089]
 * Called by Eth_17_GEthMac_Receive() for each received frame.
 * Routes to upper layer (SoAd) based on EtherType – stub logs only.
 * ====================================================================== */

FUNC(void, ETHIF_CODE)
EthIf_RxIndication(
    VAR(uint8,         AUTOMATIC) CtrlIdx,
    VAR(Eth_FrameType, AUTOMATIC) FrameType,
    VAR(boolean,       AUTOMATIC) IsBroadcast,
    P2CONST(uint8, AUTOMATIC, ETHIF_APPL_DATA) PhysAddrPtr,
    P2VAR(uint8,  AUTOMATIC, ETHIF_APPL_DATA)  DataPtr,
    VAR(uint16,        AUTOMATIC) LenByte)
{
    (void)CtrlIdx;
    (void)IsBroadcast;
    (void)PhysAddrPtr;
    (void)DataPtr;
    printf("[EthIf] RxIndication: EtherType=0x%04X  len=%u\n",
           (unsigned)FrameType, (unsigned)LenByte);
}

/* =========================================================================
 * EthIf_TxConfirmation                                 [SWS_EthIf_00090]
 * ====================================================================== */

FUNC(void, ETHIF_CODE)
EthIf_TxConfirmation(
    VAR(uint8,          AUTOMATIC) CtrlIdx,
    VAR(Eth_BufIdxType, AUTOMATIC) BufIdx)
{
    (void)CtrlIdx;
    (void)BufIdx;
    /* In production: notify SoAd_TxConfirmation() */
}

/* =========================================================================
 * EthIf_MainFunctionRx / EthIf_MainFunctionTx   [SWS_EthIf_00098/99]
 * Called from the 10 ms BSW cyclic task.
 * ====================================================================== */

FUNC(void, ETHIF_CODE) EthIf_MainFunctionRx(void)
{
    Eth_RxStatusType rxStatus = ETH_NOT_RECEIVED;
    Eth_17_GEthMac_Receive(ETH_17_GETH_MAC_CTRL_IDX_0, &rxStatus);
}

FUNC(void, ETHIF_CODE) EthIf_MainFunctionTx(void)
{
    Eth_17_GEthMac_TxConfirmation(ETH_17_GETH_MAC_CTRL_IDX_0);
}

/* =========================================================================
 * EthIf_GetVersionInfo                                 [SWS_EthIf_00095]
 * ====================================================================== */

FUNC(void, ETHIF_CODE)
EthIf_GetVersionInfo(
    P2VAR(Std_VersionInfoType, AUTOMATIC, ETHIF_APPL_DATA) VersionInfoPtr)
{
    if (VersionInfoPtr == NULL) { return; }
    VersionInfoPtr->vendorID         = ETHIF_VENDOR_ID;
    VersionInfoPtr->moduleID         = ETHIF_MODULE_ID;
    VersionInfoPtr->sw_major_version = ETHIF_SW_MAJOR_VERSION;
    VersionInfoPtr->sw_minor_version = ETHIF_SW_MINOR_VERSION;
    VersionInfoPtr->sw_patch_version = ETHIF_SW_PATCH_VERSION;
}
