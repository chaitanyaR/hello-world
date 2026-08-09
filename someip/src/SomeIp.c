/**
 * SomeIp.c – SOME/IP Transformer Module Implementation
 * AUTOSAR Release 22-11
 * Reference: AUTOSAR_SWS_SOMEIPTransformer R22-11,
 *            AUTOSAR_PRS_SOMEIPProtocol R22-11
 *
 * AUTOSAR compliance:
 *   - Module lifecycle: Init guard checked before every service function.
 *   - DET reporting:    All parameter violations reported via Det_ReportError.
 *   - Types:            AUTOSAR base types (uint8/uint16/uint32/boolean).
 *   - Memory:           No heap allocation; all buffers caller-supplied.
 *   - Naming:           Functions SomeIp_<Name>, internal SOMEIP_LOCAL static.
 */

#include "SomeIp.h"
#include "SomeIp_Cbk.h"
#include "Det.h"
#include <string.h>

/* =========================================================================
 * Internal linkage macro (AUTOSAR SWS_BSW_00301)
 * ========================================================================= */
#define SOMEIP_LOCAL    static

/* =========================================================================
 * Module state
 * ========================================================================= */
SOMEIP_LOCAL SomeIp_StateType SomeIp_State = SOMEIP_STATE_UNINIT;

/* =========================================================================
 * Internal helpers – big-endian I/O
 * ========================================================================= */

SOMEIP_LOCAL void SomeIp_WriteU16Be(uint8* BufPtr, uint16 Value)
{
    BufPtr[0] = (uint8)(Value >> 8u);
    BufPtr[1] = (uint8)(Value & 0xFFu);
}

SOMEIP_LOCAL void SomeIp_WriteU32Be(uint8* BufPtr, uint32 Value)
{
    BufPtr[0] = (uint8)(Value >> 24u);
    BufPtr[1] = (uint8)(Value >> 16u);
    BufPtr[2] = (uint8)(Value >>  8u);
    BufPtr[3] = (uint8)(Value  & 0xFFu);
}

SOMEIP_LOCAL uint16 SomeIp_ReadU16Be(const uint8* BufPtr)
{
    return (uint16)(((uint16)BufPtr[0] << 8u) | (uint16)BufPtr[1]);
}

SOMEIP_LOCAL uint32 SomeIp_ReadU32Be(const uint8* BufPtr)
{
    return ((uint32)BufPtr[0] << 24u)
         | ((uint32)BufPtr[1] << 16u)
         | ((uint32)BufPtr[2] <<  8u)
         |  (uint32)BufPtr[3];
}

/* =========================================================================
 * SomeIp_Init  (SWS_SomeIp_00001)
 * ========================================================================= */

void SomeIp_Init(const SomeIp_ConfigType* ConfigPtr)
{
    (void)ConfigPtr;    /* Default configuration used; parameter reserved */
    SomeIp_State = SOMEIP_STATE_INIT;
}

/* =========================================================================
 * SomeIp_GetVersionInfo  (SWS_SomeIp_00002)
 * ========================================================================= */

#if (SOMEIP_VERSION_INFO_API == STD_ON)
void SomeIp_GetVersionInfo(Std_VersionInfoType* VersioninfoPtr)
{
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (VersioninfoPtr == NULL)
    {
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_GET_VERSION_INFO, SOMEIP_E_NULL_PTR);
        return;
    }
#endif

    VersioninfoPtr->vendorID         = SOMEIP_VENDOR_ID;
    VersioninfoPtr->moduleID         = SOMEIP_MODULE_ID;
    VersioninfoPtr->sw_major_version = SOMEIP_SW_MAJOR_VERSION;
    VersioninfoPtr->sw_minor_version = SOMEIP_SW_MINOR_VERSION;
    VersioninfoPtr->sw_patch_version = SOMEIP_SW_PATCH_VERSION;
}
#endif /* SOMEIP_VERSION_INFO_API */

/* =========================================================================
 * SomeIp_MainFunction  (SWS_SomeIp_00003)
 * ========================================================================= */

void SomeIp_MainFunction(void)
{
    /* Session-ID housekeeping and deferred PDU handling would be placed here.
     * No cyclic work in this baseline implementation. */
}

/* =========================================================================
 * SomeIp_RxIndication  (SWS_SomeIp_00010)
 * ========================================================================= */

void SomeIp_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr)
{
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIp_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_RX_INDICATION, SOMEIP_E_UNINIT);
        return;
    }
    if (PduInfoPtr == NULL)
    {
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_RX_INDICATION, SOMEIP_E_NULL_PTR);
        return;
    }
#endif
    (void)RxPduId;
    /* Upper-layer dispatch would be implemented here in a full stack. */
}

/* =========================================================================
 * SomeIp_TxConfirmation  (SWS_SomeIp_00011)
 * ========================================================================= */

void SomeIp_TxConfirmation(PduIdType TxPduId, Std_ReturnType Result)
{
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIp_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_TX_CONFIRMATION, SOMEIP_E_UNINIT);
        return;
    }
#endif
    (void)TxPduId;
    (void)Result;
    /* Retransmit state-machine notification would go here. */
}

/* =========================================================================
 * SomeIp_ValidateHeader  (internal + public utility)
 * ========================================================================= */

Std_ReturnType SomeIp_ValidateHeader(const SomeIp_HeaderType* HeaderPtr)
{
    Std_ReturnType RetVal = E_OK;

#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIp_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_VALIDATE_HEADER, SOMEIP_E_UNINIT);
        return E_NOT_OK;
    }
    if (HeaderPtr == NULL)
    {
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_VALIDATE_HEADER, SOMEIP_E_NULL_PTR);
        return E_NOT_OK;
    }
#else
    if (HeaderPtr == NULL)
    {
        return E_NOT_OK;
    }
#endif

    /* PRS_SOMEIP_00052 – protocol version must be 0x01 */
    if (HeaderPtr->ProtocolVersion != SOMEIP_PROTOCOL_VERSION)
    {
        RetVal = E_NOT_OK;
    }

    /* PRS_SOMEIP_00030 – Length covers remaining 8 bytes of header at minimum */
    if (HeaderPtr->Length < 8u)
    {
        RetVal = E_NOT_OK;
    }

    /* PRS_SOMEIP_00055 – Message Type must be a defined value */
    if (RetVal == E_OK)
    {
        switch (HeaderPtr->MessageType)
        {
            case SOMEIP_MSG_REQUEST:
            case SOMEIP_MSG_REQUEST_NO_RETURN:
            case SOMEIP_MSG_NOTIFICATION:
            case SOMEIP_MSG_RESPONSE:
            case SOMEIP_MSG_ERROR:
            case SOMEIP_MSG_TP_REQUEST:
            case SOMEIP_MSG_TP_REQUEST_NO_RETURN:
            case SOMEIP_MSG_TP_NOTIFICATION:
            case SOMEIP_MSG_TP_RESPONSE:
            case SOMEIP_MSG_TP_ERROR:
                /* Valid */
                break;
            default:
                RetVal = E_NOT_OK;
                break;
        }
    }

    return RetVal;
}

/* =========================================================================
 * SomeIp_Serialize  (PRS_SOMEIP_00030)
 * ========================================================================= */

Std_ReturnType SomeIp_Serialize(
    const SomeIp_HeaderType* HeaderPtr,
    const PduInfoType*       PayloadPtr,
    uint8*                   BufPtr,
    uint32*                  BufLenPtr)
{
    uint32         PayloadLength;
    uint32         TotalLength;

#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIp_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_SERIALIZE, SOMEIP_E_UNINIT);
        return E_NOT_OK;
    }
    if ((HeaderPtr == NULL) || (BufPtr == NULL) || (BufLenPtr == NULL))
    {
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_SERIALIZE, SOMEIP_E_NULL_PTR);
        return E_NOT_OK;
    }
#else
    if ((HeaderPtr == NULL) || (BufPtr == NULL) || (BufLenPtr == NULL))
    {
        return E_NOT_OK;
    }
#endif

    PayloadLength = (PayloadPtr != NULL) ? PayloadPtr->SduLength : 0u;
    TotalLength   = SOMEIP_HEADER_SIZE + PayloadLength;

    if (*BufLenPtr < TotalLength)
    {
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_SERIALIZE, SOMEIP_E_BUFF_TOO_SMALL);
#endif
        return E_NOT_OK;
    }

    /* Encode header fields – big-endian (PRS_SOMEIP_00030) */
    SomeIp_WriteU16Be(&BufPtr[0],  HeaderPtr->ServiceId);
    SomeIp_WriteU16Be(&BufPtr[2],  HeaderPtr->MethodId);
    /* Length field = 8 (remaining header) + payload length */
    SomeIp_WriteU32Be(&BufPtr[4],  8u + PayloadLength);
    SomeIp_WriteU16Be(&BufPtr[8],  HeaderPtr->ClientId);
    SomeIp_WriteU16Be(&BufPtr[10], HeaderPtr->SessionId);
    BufPtr[12] = HeaderPtr->ProtocolVersion;
    BufPtr[13] = HeaderPtr->InterfaceVersion;
    BufPtr[14] = (uint8)HeaderPtr->MessageType;
    BufPtr[15] = (uint8)HeaderPtr->ReturnCode;

    /* Copy payload (if present) */
    if ((PayloadLength > 0u) && (PayloadPtr->SduDataPtr != NULL))
    {
        (void)memcpy(&BufPtr[SOMEIP_HEADER_SIZE],
                     PayloadPtr->SduDataPtr,
                     (size_t)PayloadLength);
    }

    *BufLenPtr = TotalLength;
    return E_OK;
}

/* =========================================================================
 * SomeIp_Deserialize  (PRS_SOMEIP_00030)
 * ========================================================================= */

Std_ReturnType SomeIp_Deserialize(
    const uint8*       BufPtr,
    uint32             BufLen,
    SomeIp_HeaderType* HeaderPtr,
    PduInfoType*       PayloadPtr)
{
    uint32 LengthField;
    uint32 PayloadLength;

#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIp_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_DESERIALIZE, SOMEIP_E_UNINIT);
        return E_NOT_OK;
    }
    if ((BufPtr == NULL) || (HeaderPtr == NULL) || (PayloadPtr == NULL))
    {
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_DESERIALIZE, SOMEIP_E_NULL_PTR);
        return E_NOT_OK;
    }
#else
    if ((BufPtr == NULL) || (HeaderPtr == NULL) || (PayloadPtr == NULL))
    {
        return E_NOT_OK;
    }
#endif

    /* Minimum: a complete 16-byte header must be present */
    if (BufLen < SOMEIP_HEADER_SIZE)
    {
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_DESERIALIZE, SOMEIP_E_INV_ARG);
#endif
        return E_NOT_OK;
    }

    /* Parse header fields */
    HeaderPtr->ServiceId         = SomeIp_ReadU16Be(&BufPtr[0]);
    HeaderPtr->MethodId          = SomeIp_ReadU16Be(&BufPtr[2]);
    LengthField                  = SomeIp_ReadU32Be(&BufPtr[4]);
    HeaderPtr->Length            = LengthField;
    HeaderPtr->ClientId          = SomeIp_ReadU16Be(&BufPtr[8]);
    HeaderPtr->SessionId         = SomeIp_ReadU16Be(&BufPtr[10]);
    HeaderPtr->ProtocolVersion   = BufPtr[12];
    HeaderPtr->InterfaceVersion  = BufPtr[13];
    HeaderPtr->MessageType       = (SomeIp_MessageType)BufPtr[14];
    HeaderPtr->ReturnCode        = (SomeIp_ReturnCodeType)BufPtr[15];

    /* Length field = 8 (remaining header bytes) + payload; minimum = 8 */
    if (LengthField < 8u)
    {
        return E_NOT_OK;
    }

    PayloadLength = LengthField - 8u;

    /* Ensure the buffer actually contains the advertised payload */
    if (BufLen < (SOMEIP_HEADER_SIZE + PayloadLength))
    {
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
        (void)Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID,
                              SOMEIP_SID_DESERIALIZE, SOMEIP_E_INV_ARG);
#endif
        return E_NOT_OK;
    }

    /* Zero-copy: payload pointer references the input buffer */
    if (PayloadLength > 0u)
    {
        /* Cast away const is required for PduInfoType::SduDataPtr (uint8*).
         * The caller must not modify the referenced buffer region. */
        PayloadPtr->SduDataPtr = (uint8*)&BufPtr[SOMEIP_HEADER_SIZE]; /*PRQA S 0311*/
    }
    else
    {
        PayloadPtr->SduDataPtr = NULL;
    }
    PayloadPtr->MetaDataPtr = NULL;
    PayloadPtr->SduLength   = PayloadLength;

    return SomeIp_ValidateHeader(HeaderPtr);
}

/* =========================================================================
 * Callback stubs (SomeIp_Cbk.h)
 * ========================================================================= */

void SomeIp_SoConModeChg(SoAd_SoConIdType SoConId, uint8 Mode)
{
    (void)SoConId;
    (void)Mode;
    /* Notify upper layer about socket-connection mode change */
}

void SomeIp_LocalIpAddrAssignmentChg(SoAd_SoConIdType SoConId, uint8 State)
{
    (void)SoConId;
    (void)State;
    /* Forward event to SomeIpSd for SD re-initialisation */
}
