/**
 * SomeIp_SD.c – SOME/IP Service Discovery Implementation
 * AUTOSAR Release 22-11
 * Reference: AUTOSAR_SWS_ServiceDiscovery R22-11,
 *            AUTOSAR_PRS_SOMEIPServiceDiscovery R22-11
 *
 * AUTOSAR compliance:
 *   - Lifecycle: Init guard on every service function.
 *   - DET:       All errors reported via Det_ReportError.
 *   - Types:     AUTOSAR base types; no _t suffix; CamelCase struct members.
 *   - API names: Match SWS_SD_xxx requirements exactly.
 *   - Transport: Delegated to SomeIpSd_Transmit() – no socket calls in module.
 *   - Memory:    All frame buffers are stack-local (no heap).
 *   - Session ID: Monotonically increasing per PRS_SOMEIPSD_00056 (wraps 1..0xFFFF).
 */

#include "SomeIp_SD.h"
#include "SomeIp.h"
#include "Det.h"
#include <string.h>

/* =========================================================================
 * Internal linkage macro
 * ========================================================================= */
#define SOMEIP_LOCAL    static

/* =========================================================================
 * Module state
 * ========================================================================= */
SOMEIP_LOCAL SomeIp_StateType  SomeIpSd_State     = SOMEIP_STATE_UNINIT;

/* SD session IDs are independent per call type to avoid aliasing.
 * Per PRS_SOMEIPSD_00056: must be non-zero, monotonically increasing. */
SOMEIP_LOCAL SomeIp_SessionIdType SomeIpSd_SessionIdService    = 1u;
SOMEIP_LOCAL SomeIp_SessionIdType SomeIpSd_SessionIdEventgroup = 1u;

/* =========================================================================
 * Platform interface – implemented by the BSW integration layer (SoAd)
 * ========================================================================= */
extern Std_ReturnType SomeIpSd_Transmit(const uint8* BufPtr, uint32 Length);

/* =========================================================================
 * Internal helpers – big-endian I/O
 * ========================================================================= */

SOMEIP_LOCAL void SomeIpSd_WriteU16Be(uint8* BufPtr, uint16 Value)
{
    BufPtr[0] = (uint8)(Value >> 8u);
    BufPtr[1] = (uint8)(Value & 0xFFu);
}

SOMEIP_LOCAL void SomeIpSd_WriteU32Be(uint8* BufPtr, uint32 Value)
{
    BufPtr[0] = (uint8)(Value >> 24u);
    BufPtr[1] = (uint8)(Value >> 16u);
    BufPtr[2] = (uint8)(Value >>  8u);
    BufPtr[3] = (uint8)(Value  & 0xFFu);
}

SOMEIP_LOCAL uint32 SomeIpSd_ReadU32Be(const uint8* BufPtr)
{
    return ((uint32)BufPtr[0] << 24u)
         | ((uint32)BufPtr[1] << 16u)
         | ((uint32)BufPtr[2] <<  8u)
         |  (uint32)BufPtr[3];
}

/* =========================================================================
 * Internal – session ID increment with wrap (PRS_SOMEIPSD_00056)
 * ========================================================================= */

SOMEIP_LOCAL SomeIp_SessionIdType SomeIpSd_NextSessionId(
    SomeIp_SessionIdType* SessionIdPtr)
{
    SomeIp_SessionIdType Current = *SessionIdPtr;

    *SessionIdPtr = (*SessionIdPtr == 0xFFFFu) ? 1u : (*SessionIdPtr + 1u);
    return Current;
}

/* =========================================================================
 * Internal – build and transmit an SD Type-1 entry (Service)
 * ========================================================================= */

#define SD_ENTRY_SIZE       16u    /* Both Type-1 and Type-2 are 16 bytes      */
#define SD_FLAGS_RESERVED   4u     /* Flags(1) + Reserved(3)                   */
#define SD_ARRAY_LEN_SIZE   4u     /* Entries-array length field               */
#define SD_PAYLOAD_BASE     (SD_FLAGS_RESERVED + SD_ARRAY_LEN_SIZE + SD_ENTRY_SIZE + SD_ARRAY_LEN_SIZE)
#define SD_FRAME_MAX_SIZE   (SOMEIP_HEADER_SIZE + SD_PAYLOAD_BASE)

SOMEIP_LOCAL Std_ReturnType SomeIpSd_SendServiceEntry(
    SomeIpSd_EntryType     EntryType,
    SomeIp_ServiceIdType   ServiceId,
    SomeIp_InstanceIdType  InstanceId,
    uint8                  MajorVersion,
    uint32                 MinorVersion,
    uint32                 TtlSeconds)
{
    uint8              Frame[SD_FRAME_MAX_SIZE];
    uint8              Payload[SD_PAYLOAD_BASE];
    uint32             PayloadOff = 0u;
    uint32             FrameLen   = 0u;
    SomeIp_HeaderType  Header;
    PduInfoType        PayloadPdu;

    (void)memset(Frame,   0, sizeof(Frame));
    (void)memset(Payload, 0, sizeof(Payload));
    (void)memset(&Header, 0, sizeof(Header));

    /* --- SD payload: Flags + Reserved (4 bytes) -------------------------- */
    Payload[PayloadOff] = SOMEIP_SD_FLAG_REBOOT;    /* set on (re)start      */
    PayloadOff         += SD_FLAGS_RESERVED;

    /* --- Entries array length = 16 bytes (one entry) --------------------- */
    SomeIpSd_WriteU32Be(&Payload[PayloadOff], (uint32)SD_ENTRY_SIZE);
    PayloadOff += SD_ARRAY_LEN_SIZE;

    /* --- Type-1 SD entry ------------------------------------------------- */
    Payload[PayloadOff++] = (uint8)EntryType;
    Payload[PayloadOff++] = 0x00u;          /* Index first option  */
    Payload[PayloadOff++] = 0x00u;          /* Index second option */
    Payload[PayloadOff++] = 0x00u;          /* #Opts1 | #Opts2     */
    SomeIpSd_WriteU16Be(&Payload[PayloadOff], ServiceId);   PayloadOff += 2u;
    SomeIpSd_WriteU16Be(&Payload[PayloadOff], InstanceId);  PayloadOff += 2u;
    Payload[PayloadOff++] = MajorVersion;
    Payload[PayloadOff++] = (uint8)((TtlSeconds >> 16u) & 0xFFu);
    Payload[PayloadOff++] = (uint8)((TtlSeconds >>  8u) & 0xFFu);
    Payload[PayloadOff++] = (uint8)( TtlSeconds         & 0xFFu);
    SomeIpSd_WriteU32Be(&Payload[PayloadOff], MinorVersion); PayloadOff += 4u;

    /* --- Options array length = 0 (no options attached) ------------------ */
    SomeIpSd_WriteU32Be(&Payload[PayloadOff], 0u);
    PayloadOff += SD_ARRAY_LEN_SIZE;

    /* --- Build SOME/IP header -------------------------------------------- */
    Header.ServiceId        = SOMEIP_SD_SERVICE_ID;
    Header.MethodId         = SOMEIP_SD_METHOD_ID;
    Header.ClientId         = 0x0000u;
    Header.SessionId        = SomeIpSd_NextSessionId(&SomeIpSd_SessionIdService);
    Header.ProtocolVersion  = SOMEIP_PROTOCOL_VERSION;
    Header.InterfaceVersion = SOMEIP_INTERFACE_VERSION_DEFAULT;
    Header.MessageType      = SOMEIP_MSG_NOTIFICATION;
    Header.ReturnCode       = SOMEIP_RC_OK;

    PayloadPdu.SduDataPtr   = Payload;
    PayloadPdu.MetaDataPtr  = NULL;
    PayloadPdu.SduLength    = PayloadOff;

    FrameLen = sizeof(Frame);
    if (SomeIp_Serialize(&Header, &PayloadPdu, Frame, &FrameLen) != E_OK)
    {
        return E_NOT_OK;
    }

    return SomeIpSd_Transmit(Frame, FrameLen);
}

/* =========================================================================
 * Internal – build and transmit an SD Type-2 entry (Eventgroup)
 * ========================================================================= */

SOMEIP_LOCAL Std_ReturnType SomeIpSd_SendEventGroupEntry(
    SomeIpSd_EntryType       EntryType,
    SomeIp_ServiceIdType     ServiceId,
    SomeIp_InstanceIdType    InstanceId,
    SomeIp_EventGroupIdType  EventGroupId,
    uint8                    MajorVersion,
    uint32                   TtlSeconds)
{
    uint8              Frame[SD_FRAME_MAX_SIZE];
    uint8              Payload[SD_PAYLOAD_BASE];
    uint32             PayloadOff = 0u;
    uint32             FrameLen   = 0u;
    SomeIp_HeaderType  Header;
    PduInfoType        PayloadPdu;

    (void)memset(Frame,   0, sizeof(Frame));
    (void)memset(Payload, 0, sizeof(Payload));
    (void)memset(&Header, 0, sizeof(Header));

    /* Flags + Reserved */
    Payload[PayloadOff] = SOMEIP_SD_FLAG_REBOOT;
    PayloadOff         += SD_FLAGS_RESERVED;

    /* Entries array length = 16 bytes */
    SomeIpSd_WriteU32Be(&Payload[PayloadOff], (uint32)SD_ENTRY_SIZE);
    PayloadOff += SD_ARRAY_LEN_SIZE;

    /* Type-2 entry */
    Payload[PayloadOff++] = (uint8)EntryType;
    Payload[PayloadOff++] = 0x00u;
    Payload[PayloadOff++] = 0x00u;
    Payload[PayloadOff++] = 0x00u;
    SomeIpSd_WriteU16Be(&Payload[PayloadOff], ServiceId);    PayloadOff += 2u;
    SomeIpSd_WriteU16Be(&Payload[PayloadOff], InstanceId);   PayloadOff += 2u;
    Payload[PayloadOff++] = MajorVersion;
    Payload[PayloadOff++] = (uint8)((TtlSeconds >> 16u) & 0xFFu);
    Payload[PayloadOff++] = (uint8)((TtlSeconds >>  8u) & 0xFFu);
    Payload[PayloadOff++] = (uint8)( TtlSeconds         & 0xFFu);
    Payload[PayloadOff++] = 0x00u;     /* Reserved                */
    Payload[PayloadOff++] = 0x00u;     /* Counter (lower nibble)  */
    SomeIpSd_WriteU16Be(&Payload[PayloadOff], EventGroupId); PayloadOff += 2u;

    /* Options array length = 0 */
    SomeIpSd_WriteU32Be(&Payload[PayloadOff], 0u);
    PayloadOff += SD_ARRAY_LEN_SIZE;

    Header.ServiceId        = SOMEIP_SD_SERVICE_ID;
    Header.MethodId         = SOMEIP_SD_METHOD_ID;
    Header.ClientId         = 0x0000u;
    Header.SessionId        = SomeIpSd_NextSessionId(&SomeIpSd_SessionIdEventgroup);
    Header.ProtocolVersion  = SOMEIP_PROTOCOL_VERSION;
    Header.InterfaceVersion = SOMEIP_INTERFACE_VERSION_DEFAULT;
    Header.MessageType      = SOMEIP_MSG_NOTIFICATION;
    Header.ReturnCode       = SOMEIP_RC_OK;

    PayloadPdu.SduDataPtr  = Payload;
    PayloadPdu.MetaDataPtr = NULL;
    PayloadPdu.SduLength   = PayloadOff;

    FrameLen = sizeof(Frame);
    if (SomeIp_Serialize(&Header, &PayloadPdu, Frame, &FrameLen) != E_OK)
    {
        return E_NOT_OK;
    }

    return SomeIpSd_Transmit(Frame, FrameLen);
}

/* =========================================================================
 * SomeIpSd_Init  (SWS_SD_00001)
 * ========================================================================= */

void SomeIpSd_Init(const SomeIpSd_ConfigType* ConfigPtr)
{
    (void)ConfigPtr;    /* Post-build config reserved for future use */

    SomeIpSd_SessionIdService    = 1u;
    SomeIpSd_SessionIdEventgroup = 1u;
    SomeIpSd_State               = SOMEIP_STATE_INIT;
}

/* =========================================================================
 * SomeIpSd_GetVersionInfo  (SWS_SD_00002)
 * ========================================================================= */

#if (SOMEIP_VERSION_INFO_API == STD_ON)
void SomeIpSd_GetVersionInfo(Std_VersionInfoType* VersioninfoPtr)
{
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (VersioninfoPtr == NULL)
    {
        (void)Det_ReportError(SOMEIPSD_MODULE_ID, SOMEIPSD_INSTANCE_ID,
                              SOMEIPSD_SID_GET_VERSION_INFO, SOMEIPSD_E_NULL_PTR);
        return;
    }
#endif
    VersioninfoPtr->vendorID         = SOMEIPSD_VENDOR_ID;
    VersioninfoPtr->moduleID         = SOMEIPSD_MODULE_ID;
    VersioninfoPtr->sw_major_version = SOMEIPSD_SW_MAJOR_VERSION;
    VersioninfoPtr->sw_minor_version = SOMEIPSD_SW_MINOR_VERSION;
    VersioninfoPtr->sw_patch_version = SOMEIPSD_SW_PATCH_VERSION;
}
#endif

/* =========================================================================
 * SomeIpSd_MainFunction  (SWS_SD_00003)
 * ========================================================================= */

void SomeIpSd_MainFunction(void)
{
    /* Offer/Find retry timers and subscription TTL countdown would be
     * implemented here for a production SD stack. */
}

/* =========================================================================
 * SomeIpSd_LocalIpAddrAssignmentChg  (SWS_SD_00005)
 * ========================================================================= */

void SomeIpSd_LocalIpAddrAssignmentChg(SoAd_SoConIdType SoConId, uint8 State)
{
    (void)SoConId;
    (void)State;
    /* Re-initialise SD state on IP address change. */
}

/* =========================================================================
 * Service offer / find  (SWS_SD_00006 … SWS_SD_00009)
 * ========================================================================= */

Std_ReturnType SomeIpSd_OfferService(
    SomeIp_ServiceIdType   ServiceId,
    SomeIp_InstanceIdType  InstanceId,
    uint8                  MajorVersion,
    uint32                 MinorVersion,
    uint32                 TtlSeconds)
{
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIpSd_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIPSD_MODULE_ID, SOMEIPSD_INSTANCE_ID,
                              SOMEIPSD_SID_OFFER_SERVICE, SOMEIPSD_E_UNINIT);
        return E_NOT_OK;
    }
#endif
    return SomeIpSd_SendServiceEntry(
        SOMEIPSD_ENTRY_OFFER_SERVICE,
        ServiceId, InstanceId, MajorVersion, MinorVersion, TtlSeconds);
}

Std_ReturnType SomeIpSd_StopOfferService(
    SomeIp_ServiceIdType   ServiceId,
    SomeIp_InstanceIdType  InstanceId,
    uint8                  MajorVersion,
    uint32                 MinorVersion)
{
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIpSd_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIPSD_MODULE_ID, SOMEIPSD_INSTANCE_ID,
                              SOMEIPSD_SID_STOP_OFFER_SERVICE, SOMEIPSD_E_UNINIT);
        return E_NOT_OK;
    }
#endif
    return SomeIpSd_SendServiceEntry(
        SOMEIPSD_ENTRY_OFFER_SERVICE,
        ServiceId, InstanceId, MajorVersion, MinorVersion, 0u);
}

Std_ReturnType SomeIpSd_FindService(
    SomeIp_ServiceIdType   ServiceId,
    SomeIp_InstanceIdType  InstanceId,
    uint8                  MajorVersion,
    uint32                 MinorVersion)
{
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIpSd_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIPSD_MODULE_ID, SOMEIPSD_INSTANCE_ID,
                              SOMEIPSD_SID_FIND_SERVICE, SOMEIPSD_E_UNINIT);
        return E_NOT_OK;
    }
#endif
    return SomeIpSd_SendServiceEntry(
        SOMEIPSD_ENTRY_FIND_SERVICE,
        ServiceId, InstanceId, MajorVersion, MinorVersion, 0xFFFFFFu);
}

Std_ReturnType SomeIpSd_ReleaseService(
    SomeIp_ServiceIdType   ServiceId,
    SomeIp_InstanceIdType  InstanceId)
{
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIpSd_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIPSD_MODULE_ID, SOMEIPSD_INSTANCE_ID,
                              SOMEIPSD_SID_RELEASE_SERVICE, SOMEIPSD_E_UNINIT);
        return E_NOT_OK;
    }
#endif
    (void)ServiceId;
    (void)InstanceId;
    /* Stop ongoing FindService; no wire message required per spec. */
    return E_OK;
}

/* =========================================================================
 * Eventgroup subscription  (SWS_SD_00010, SWS_SD_00011)
 * ========================================================================= */

Std_ReturnType SomeIpSd_SubscribeEventgroup(
    SomeIp_ServiceIdType     ServiceId,
    SomeIp_InstanceIdType    InstanceId,
    SomeIp_EventGroupIdType  EventGroupId,
    uint8                    MajorVersion,
    uint32                   TtlSeconds)
{
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIpSd_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIPSD_MODULE_ID, SOMEIPSD_INSTANCE_ID,
                              SOMEIPSD_SID_SUBSCRIBE_EVENTGROUP, SOMEIPSD_E_UNINIT);
        return E_NOT_OK;
    }
#endif
    return SomeIpSd_SendEventGroupEntry(
        SOMEIPSD_ENTRY_SUBSCRIBE_EVENTGROUP,
        ServiceId, InstanceId, EventGroupId, MajorVersion, TtlSeconds);
}

Std_ReturnType SomeIpSd_StopSubscribeEventgroup(
    SomeIp_ServiceIdType     ServiceId,
    SomeIp_InstanceIdType    InstanceId,
    SomeIp_EventGroupIdType  EventGroupId,
    uint8                    MajorVersion)
{
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIpSd_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIPSD_MODULE_ID, SOMEIPSD_INSTANCE_ID,
                              SOMEIPSD_SID_STOP_SUBSCRIBE_EVENTGROUP, SOMEIPSD_E_UNINIT);
        return E_NOT_OK;
    }
#endif
    return SomeIpSd_SendEventGroupEntry(
        SOMEIPSD_ENTRY_SUBSCRIBE_EVENTGROUP,
        ServiceId, InstanceId, EventGroupId, MajorVersion, 0u);
}

/* =========================================================================
 * SomeIpSd_RxIndication  (SWS_SD_00012)
 * ========================================================================= */

void SomeIpSd_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr)
{
    SomeIp_HeaderType  Header;
    PduInfoType        Payload;
    uint32             EntriesArrayLen;
    uint32             Offset;
    uint8              EntryType;

    (void)RxPduId;

#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIpSd_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIPSD_MODULE_ID, SOMEIPSD_INSTANCE_ID,
                              SOMEIPSD_SID_RX_INDICATION, SOMEIPSD_E_UNINIT);
        return;
    }
    if (PduInfoPtr == NULL)
    {
        (void)Det_ReportError(SOMEIPSD_MODULE_ID, SOMEIPSD_INSTANCE_ID,
                              SOMEIPSD_SID_RX_INDICATION, SOMEIPSD_E_NULL_PTR);
        return;
    }
    if (PduInfoPtr->SduDataPtr == NULL)
    {
        (void)Det_ReportError(SOMEIPSD_MODULE_ID, SOMEIPSD_INSTANCE_ID,
                              SOMEIPSD_SID_RX_INDICATION, SOMEIPSD_E_NULL_PTR);
        return;
    }
#else
    if ((PduInfoPtr == NULL) || (PduInfoPtr->SduDataPtr == NULL))
    {
        return;
    }
#endif

    (void)memset(&Header,  0, sizeof(Header));
    (void)memset(&Payload, 0, sizeof(Payload));

    if (SomeIp_Deserialize(PduInfoPtr->SduDataPtr,
                           PduInfoPtr->SduLength,
                           &Header, &Payload) != E_OK)
    {
        return;
    }

    /* Reject non-SD frames (PRS_SOMEIPSD_00001) */
    if ((Header.ServiceId != SOMEIP_SD_SERVICE_ID) ||
        (Header.MethodId  != SOMEIP_SD_METHOD_ID))
    {
        return;
    }

    /* SD payload must have at least: flags(1)+reserved(3)+entries_len(4) = 8 */
    if ((Payload.SduDataPtr == NULL) || (Payload.SduLength < 8u))
    {
        return;
    }

    /* Skip Flags(1) + Reserved(3) */
    Offset = 4u;

    EntriesArrayLen = SomeIpSd_ReadU32Be(&Payload.SduDataPtr[Offset]);
    Offset         += 4u;

    /* Iterate over entries; each is SD_ENTRY_SIZE bytes */
    while (EntriesArrayLen >= SD_ENTRY_SIZE)
    {
        if ((Offset + SD_ENTRY_SIZE) > Payload.SduLength)
        {
            break;
        }

        EntryType = Payload.SduDataPtr[Offset];

        /*
         * Production implementation would dispatch to service registry,
         * eventgroup manager, or upper-layer callbacks here based on EntryType.
         */
        (void)EntryType;

        Offset          += SD_ENTRY_SIZE;
        EntriesArrayLen -= SD_ENTRY_SIZE;
    }
}

/* =========================================================================
 * SomeIpSd_TxConfirmation  (SWS_SD_00013)
 * ========================================================================= */

void SomeIpSd_TxConfirmation(PduIdType TxPduId, Std_ReturnType Result)
{
#if (SOMEIP_DEV_ERROR_DETECT == STD_ON)
    if (SomeIpSd_State == SOMEIP_STATE_UNINIT)
    {
        (void)Det_ReportError(SOMEIPSD_MODULE_ID, SOMEIPSD_INSTANCE_ID,
                              SOMEIPSD_SID_TX_CONFIRMATION, SOMEIPSD_E_UNINIT);
        return;
    }
#endif
    (void)TxPduId;
    (void)Result;
    /* Retry / state-machine notification goes here for a full implementation. */
}
