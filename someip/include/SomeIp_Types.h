/**
 * SomeIp_Types.h – SOME/IP Module Type Definitions
 * AUTOSAR Release 22-11
 * Reference: AUTOSAR_SWS_SOMEIPTransformer, AUTOSAR_PRS_SOMEIPProtocol
 *
 * All types shared between the SOME/IP transformer (SomeIp) and the
 * Service Discovery (SomeIpSd) modules are defined here.
 *
 * AUTOSAR naming rules applied:
 *   Types        : <Module>_<Name>Type    (e.g. SomeIp_MessageType)
 *   Struct fields: CamelCase              (e.g. ServiceId, MethodId)
 *   Constants    : SOMEIP_<NAME>          (e.g. SOMEIP_PROTOCOL_VERSION)
 *   No "_t" suffix (POSIX/C99 convention, not AUTOSAR).
 */

#ifndef SOMEIP_TYPES_H
#define SOMEIP_TYPES_H

/* ----- Version check ----------------------------------------------------- */
#define SOMEIP_TYPES_AR_RELEASE_MAJOR_VERSION   22u
#define SOMEIP_TYPES_AR_RELEASE_MINOR_VERSION   11u
#define SOMEIP_TYPES_AR_RELEASE_PATCH_VERSION    0u

#include "Std_Types.h"
#include "ComStack_Types.h"

/* =========================================================================
 * Identifier types  (PRS_SOMEIP_00030)
 * ========================================================================= */

typedef uint16 SomeIp_ServiceIdType;
typedef uint16 SomeIp_MethodIdType;
typedef uint16 SomeIp_ClientIdType;
typedef uint16 SomeIp_SessionIdType;
typedef uint16 SomeIp_InstanceIdType;
typedef uint16 SomeIp_EventIdType;
typedef uint16 SomeIp_EventGroupIdType;

/* =========================================================================
 * Constants
 * ========================================================================= */

/** Fixed protocol version mandated by PRS_SOMEIP_00052 */
#define SOMEIP_PROTOCOL_VERSION         0x01u

/** Default interface (major) version – application overrides per service */
#define SOMEIP_INTERFACE_VERSION_DEFAULT 0x01u

/** SOME/IP header size in bytes (PRS_SOMEIP_00030) */
#define SOMEIP_HEADER_SIZE              16u

/** Service Discovery fixed Service ID and Method ID (PRS_SOMEIPSD_00001) */
#define SOMEIP_SD_SERVICE_ID            ((SomeIp_ServiceIdType)0xFFFFu)
#define SOMEIP_SD_METHOD_ID             ((SomeIp_MethodIdType)0x8100u)

/** SD flag bits (PRS_SOMEIPSD_00049) */
#define SOMEIP_SD_FLAG_REBOOT           0x80u
#define SOMEIP_SD_FLAG_UNICAST          0x40u

/** Wildcard values for FindService */
#define SOMEIP_INSTANCE_ID_ANY          ((SomeIp_InstanceIdType)0xFFFFu)
#define SOMEIP_MAJOR_VERSION_ANY        0xFFu
#define SOMEIP_MINOR_VERSION_ANY        0xFFFFFFFFu

/* =========================================================================
 * Message Type  (PRS_SOMEIP_00055)
 * ========================================================================= */

typedef enum {
    SOMEIP_MSG_REQUEST              = 0x00u,
    SOMEIP_MSG_REQUEST_NO_RETURN    = 0x01u,
    SOMEIP_MSG_NOTIFICATION         = 0x02u,
    SOMEIP_MSG_RESPONSE             = 0x80u,
    SOMEIP_MSG_ERROR                = 0x81u,
    SOMEIP_MSG_TP_REQUEST           = 0x20u,
    SOMEIP_MSG_TP_REQUEST_NO_RETURN = 0x21u,
    SOMEIP_MSG_TP_NOTIFICATION      = 0x22u,
    SOMEIP_MSG_TP_RESPONSE          = 0xA0u,
    SOMEIP_MSG_TP_ERROR             = 0xA1u
} SomeIp_MessageType;

/* =========================================================================
 * Return Code  (PRS_SOMEIP_00058)
 * ========================================================================= */

typedef enum {
    SOMEIP_RC_OK                        = 0x00u,
    SOMEIP_RC_NOT_OK                    = 0x01u,
    SOMEIP_RC_UNKNOWN_SERVICE           = 0x02u,
    SOMEIP_RC_UNKNOWN_METHOD            = 0x03u,
    SOMEIP_RC_NOT_READY                 = 0x04u,
    SOMEIP_RC_NOT_REACHABLE             = 0x05u,
    SOMEIP_RC_TIMEOUT                   = 0x06u,
    SOMEIP_RC_WRONG_PROTOCOL_VERSION    = 0x07u,
    SOMEIP_RC_WRONG_INTERFACE_VERSION   = 0x08u,
    SOMEIP_RC_MALFORMED_MESSAGE         = 0x09u,
    SOMEIP_RC_WRONG_MESSAGE_TYPE        = 0x0Au
} SomeIp_ReturnCodeType;

/* =========================================================================
 * SOME/IP Header  (PRS_SOMEIP_00030)
 * ========================================================================= */

typedef struct {
    SomeIp_ServiceIdType   ServiceId;
    SomeIp_MethodIdType    MethodId;
    uint32                 Length;          /**< Payload length + 8 bytes */
    SomeIp_ClientIdType    ClientId;
    SomeIp_SessionIdType   SessionId;
    uint8                  ProtocolVersion; /**< Always SOMEIP_PROTOCOL_VERSION */
    uint8                  InterfaceVersion;
    SomeIp_MessageType     MessageType;
    SomeIp_ReturnCodeType  ReturnCode;
} SomeIp_HeaderType;

/* =========================================================================
 * Service Discovery entry types  (PRS_SOMEIPSD_00009)
 * ========================================================================= */

typedef enum {
    SOMEIPSD_ENTRY_FIND_SERVICE             = 0x00u,
    SOMEIPSD_ENTRY_OFFER_SERVICE            = 0x01u,
    SOMEIPSD_ENTRY_SUBSCRIBE_EVENTGROUP     = 0x06u,
    SOMEIPSD_ENTRY_SUBSCRIBE_EVENTGROUP_ACK = 0x07u
} SomeIpSd_EntryType;

/* SD Type-1 entry (Find / Offer Service) */
typedef struct {
    SomeIpSd_EntryType       Type;
    uint8                    IndexFirstOption;
    uint8                    IndexSecondOption;
    uint8                    NumberOfOptions;   /**< packed nibble pair */
    SomeIp_ServiceIdType     ServiceId;
    SomeIp_InstanceIdType    InstanceId;
    uint8                    MajorVersion;
    uint8                    Ttl[3u];            /**< 24-bit TTL, big-endian */
    uint32                   MinorVersion;
} SomeIpSd_ServiceEntryType;

/* SD Type-2 entry (Subscribe / Subscribe-Ack Eventgroup) */
typedef struct {
    SomeIpSd_EntryType       Type;
    uint8                    IndexFirstOption;
    uint8                    IndexSecondOption;
    uint8                    NumberOfOptions;
    SomeIp_ServiceIdType     ServiceId;
    SomeIp_InstanceIdType    InstanceId;
    uint8                    MajorVersion;
    uint8                    Ttl[3u];
    uint8                    Reserved;
    uint8                    Counter;            /**< lower nibble only */
    SomeIp_EventGroupIdType  EventGroupId;
} SomeIpSd_EventGroupEntryType;

/* =========================================================================
 * Server service availability state (SWS_SD_00381)
 * ========================================================================= */

typedef enum {
    SOMEIPSD_SERVICE_DOWN      = 0x00u,
    SOMEIPSD_SERVICE_AVAILABLE = 0x01u
} SomeIpSd_ServiceStateType;

/* =========================================================================
 * Module state
 * ========================================================================= */

typedef enum {
    SOMEIP_STATE_UNINIT = 0x00u,
    SOMEIP_STATE_INIT   = 0x01u
} SomeIp_StateType;

/* =========================================================================
 * Configuration types
 * ========================================================================= */

/** Pre-compile / post-build configuration container for SomeIp */
typedef struct {
    uint8 ProtocolVersion;      /**< Expected protocol version (normally 0x01) */
} SomeIp_ConfigType;

/** Pre-compile / post-build configuration container for SomeIpSd */
typedef struct {
    uint32 InitialDelayMinMs;   /**< Minimum initial delay (ms) */
    uint32 InitialDelayMaxMs;   /**< Maximum initial delay (ms) */
    uint32 CyclicOfferDelayMs;  /**< Cyclic offer period (ms)  */
} SomeIpSd_ConfigType;

#endif /* SOMEIP_TYPES_H */
