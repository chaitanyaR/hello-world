/**
 * SomeIp_SD.h – SOME/IP Service Discovery Module API
 * AUTOSAR Release 22-11
 * Reference: AUTOSAR_SWS_ServiceDiscovery R22-11,
 *            AUTOSAR_PRS_SOMEIPServiceDiscovery R22-11
 *
 * Provides the public API of the SOME/IP Service Discovery module:
 *   - Lifecycle  : SomeIpSd_Init, SomeIpSd_GetVersionInfo, SomeIpSd_MainFunction
 *   - Service    : SomeIpSd_OfferService, SomeIpSd_StopOfferService,
 *                  SomeIpSd_FindService, SomeIpSd_ReleaseService
 *   - Eventgroup : SomeIpSd_SubscribeEventgroup, SomeIpSd_StopSubscribeEventgroup
 *   - Indication : SomeIpSd_RxIndication, SomeIpSd_TxConfirmation
 */

#ifndef SOMEIP_SD_H
#define SOMEIP_SD_H

#include "SomeIp_Types.h"
#include "SomeIp_Cfg.h"

/* ----- Vendor / Module identification ------------------------------------ */
#define SOMEIPSD_VENDOR_ID      0x0000u
#define SOMEIPSD_MODULE_ID      0x00AAu     /**< AUTOSAR BSW Module ID 170 */
#define SOMEIPSD_INSTANCE_ID    0x00u

/* ----- AUTOSAR release version ------------------------------------------- */
#define SOMEIPSD_AR_RELEASE_MAJOR_VERSION   22u
#define SOMEIPSD_AR_RELEASE_MINOR_VERSION   11u
#define SOMEIPSD_AR_RELEASE_PATCH_VERSION    0u

/* ----- Software version -------------------------------------------------- */
#define SOMEIPSD_SW_MAJOR_VERSION   1u
#define SOMEIPSD_SW_MINOR_VERSION   0u
#define SOMEIPSD_SW_PATCH_VERSION   0u

/* ----- Version consistency checks ---------------------------------------- */
#if (SOMEIP_TYPES_AR_RELEASE_MAJOR_VERSION != SOMEIPSD_AR_RELEASE_MAJOR_VERSION)
#  error "SomeIp_Types.h: AR major version mismatch with SomeIp_SD.h"
#endif

/* =========================================================================
 * Development Error IDs
 * ========================================================================= */
#define SOMEIPSD_E_NULL_PTR             0x01u
#define SOMEIPSD_E_INV_ARG              0x02u
#define SOMEIPSD_E_UNINIT               0x03u
#define SOMEIPSD_E_INV_PDU_SDU_ID       0x04u

/* =========================================================================
 * API Service IDs
 * ========================================================================= */
#define SOMEIPSD_SID_INIT                       0x01u
#define SOMEIPSD_SID_GET_VERSION_INFO           0x02u
#define SOMEIPSD_SID_MAIN_FUNCTION              0x03u
#define SOMEIPSD_SID_LOCAL_IP_ADDR_ASSIGN_CHG   0x05u
#define SOMEIPSD_SID_RX_INDICATION              0x06u
#define SOMEIPSD_SID_TX_CONFIRMATION            0x07u
#define SOMEIPSD_SID_OFFER_SERVICE              0x10u
#define SOMEIPSD_SID_STOP_OFFER_SERVICE         0x11u
#define SOMEIPSD_SID_FIND_SERVICE               0x12u
#define SOMEIPSD_SID_RELEASE_SERVICE            0x13u
#define SOMEIPSD_SID_SUBSCRIBE_EVENTGROUP       0x14u
#define SOMEIPSD_SID_STOP_SUBSCRIBE_EVENTGROUP  0x15u

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

/**
 * SomeIpSd_Init
 * Initialises the SOME/IP Service Discovery module.
 * Must be called before any other SomeIpSd_* function.
 *
 * @param ConfigPtr  Pointer to module configuration. NULL uses defaults.
 */
void SomeIpSd_Init(const SomeIpSd_ConfigType* ConfigPtr);

/**
 * SomeIpSd_GetVersionInfo
 * Returns the version information of the SomeIpSd module.
 */
#if (SOMEIP_VERSION_INFO_API == STD_ON)
void SomeIpSd_GetVersionInfo(Std_VersionInfoType* VersioninfoPtr);
#endif

/**
 * SomeIpSd_MainFunction
 * Cyclic processing function (timing configured by system integrator).
 * Handles offer timers, retry logic, and subscription TTL management.
 */
void SomeIpSd_MainFunction(void);

/**
 * SomeIpSd_LocalIpAddrAssignmentChg
 * Called by SoAd when the local IP address assignment state changes.
 *
 * @param SoConId  Socket connection identifier.
 * @param State    New IP address assignment state.
 */
void SomeIpSd_LocalIpAddrAssignmentChg(SoAd_SoConIdType SoConId, uint8 State);

/* =========================================================================
 * Service offer / find
 * ========================================================================= */

/**
 * SomeIpSd_OfferService
 * Transmits an SD Offer Service entry (Type 0x01) for the given service.
 *
 * @param ServiceId    Service identifier.
 * @param InstanceId   Instance identifier.
 * @param MajorVersion Service major version.
 * @param MinorVersion Service minor version.
 * @param TtlSeconds   Time-to-live in seconds. 0 = Stop Offer (PRS_SOMEIPSD_00008).
 * @return E_OK / E_NOT_OK.
 */
Std_ReturnType SomeIpSd_OfferService(
    SomeIp_ServiceIdType   ServiceId,
    SomeIp_InstanceIdType  InstanceId,
    uint8                  MajorVersion,
    uint32                 MinorVersion,
    uint32                 TtlSeconds);

/**
 * SomeIpSd_StopOfferService
 * Transmits an SD Stop Offer Service entry (Offer with TTL = 0).
 */
Std_ReturnType SomeIpSd_StopOfferService(
    SomeIp_ServiceIdType   ServiceId,
    SomeIp_InstanceIdType  InstanceId,
    uint8                  MajorVersion,
    uint32                 MinorVersion);

/**
 * SomeIpSd_FindService
 * Transmits an SD Find Service entry (Type 0x00).
 * Wildcard values: SOMEIP_INSTANCE_ID_ANY, SOMEIP_MAJOR_VERSION_ANY,
 * SOMEIP_MINOR_VERSION_ANY.
 */
Std_ReturnType SomeIpSd_FindService(
    SomeIp_ServiceIdType   ServiceId,
    SomeIp_InstanceIdType  InstanceId,
    uint8                  MajorVersion,
    uint32                 MinorVersion);

/**
 * SomeIpSd_ReleaseService
 * Stops finding a previously requested service.
 */
Std_ReturnType SomeIpSd_ReleaseService(
    SomeIp_ServiceIdType   ServiceId,
    SomeIp_InstanceIdType  InstanceId);

/* =========================================================================
 * Eventgroup subscription
 * ========================================================================= */

/**
 * SomeIpSd_SubscribeEventgroup
 * Transmits an SD Subscribe Eventgroup entry (Type 0x06).
 *
 * @param TtlSeconds  0 = Unsubscribe (PRS_SOMEIPSD_00008).
 */
Std_ReturnType SomeIpSd_SubscribeEventgroup(
    SomeIp_ServiceIdType     ServiceId,
    SomeIp_InstanceIdType    InstanceId,
    SomeIp_EventGroupIdType  EventGroupId,
    uint8                    MajorVersion,
    uint32                   TtlSeconds);

/**
 * SomeIpSd_StopSubscribeEventgroup
 * Transmits an SD Subscribe Eventgroup with TTL = 0 (Unsubscribe).
 */
Std_ReturnType SomeIpSd_StopSubscribeEventgroup(
    SomeIp_ServiceIdType     ServiceId,
    SomeIp_InstanceIdType    InstanceId,
    SomeIp_EventGroupIdType  EventGroupId,
    uint8                    MajorVersion);

/* =========================================================================
 * Lower-layer interface (called by SoAd)
 * ========================================================================= */

/**
 * SomeIpSd_RxIndication
 * Called by SoAd when an SD frame has been received from the network.
 *
 * @param RxPduId     Receive PDU identifier (from PduR routing table).
 * @param PduInfoPtr  PDU data pointer and length.
 */
void SomeIpSd_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr);

/**
 * SomeIpSd_TxConfirmation
 * Called by SoAd after an SD frame has been transmitted.
 *
 * @param TxPduId  Transmit PDU identifier.
 * @param Result   E_OK: success; E_NOT_OK: failure.
 */
void SomeIpSd_TxConfirmation(PduIdType TxPduId, Std_ReturnType Result);

#ifdef __cplusplus
}
#endif

#endif /* SOMEIP_SD_H */
