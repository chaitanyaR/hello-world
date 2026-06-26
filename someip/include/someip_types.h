/**
 * SOME/IP (Scalable service-Oriented MiddlewarE over IP)
 * Type definitions per AUTOSAR PRS_SOMEIPProtocol specification.
 *
 * References:
 *   - AUTOSAR PRS_SOMEIPProtocol R22-11
 *   - AUTOSAR SWS_SOMEIPTransformer
 */

#ifndef SOMEIP_TYPES_H
#define SOMEIP_TYPES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 * SOME/IP header field sizes (bytes)
 * ----------------------------------------------------------------------- */
#define SOMEIP_HEADER_SIZE          16u
#define SOMEIP_SD_HEADER_SIZE       12u

/* -----------------------------------------------------------------------
 * Magic cookie values (AUTOSAR PRS_SOMEIP_00010)
 * ----------------------------------------------------------------------- */
#define SOMEIP_MAGIC_COOKIE_CLIENT  0xFFFF8001u
#define SOMEIP_MAGIC_COOKIE_SERVER  0xFFFF8002u

/* -----------------------------------------------------------------------
 * Reserved / fixed field values
 * ----------------------------------------------------------------------- */
#define SOMEIP_PROTOCOL_VERSION     0x01u
#define SOMEIP_INTERFACE_VERSION    0x01u

/* -----------------------------------------------------------------------
 * Message types (AUTOSAR PRS_SOMEIP_00055)
 * ----------------------------------------------------------------------- */
typedef enum {
    SOMEIP_MSG_REQUEST              = 0x00,
    SOMEIP_MSG_REQUEST_NO_RETURN    = 0x01,
    SOMEIP_MSG_NOTIFICATION         = 0x02,
    SOMEIP_MSG_RESPONSE             = 0x80,
    SOMEIP_MSG_ERROR                = 0x81,
    SOMEIP_MSG_TP_REQUEST           = 0x20,
    SOMEIP_MSG_TP_REQUEST_NO_RETURN = 0x21,
    SOMEIP_MSG_TP_NOTIFICATION      = 0x22,
    SOMEIP_MSG_TP_RESPONSE          = 0xA0,
    SOMEIP_MSG_TP_ERROR             = 0xA1
} SomeIp_MessageType_t;

/* -----------------------------------------------------------------------
 * Return codes (AUTOSAR PRS_SOMEIP_00058)
 * ----------------------------------------------------------------------- */
typedef enum {
    SOMEIP_RC_OK                         = 0x00,
    SOMEIP_RC_NOT_OK                     = 0x01,
    SOMEIP_RC_UNKNOWN_SERVICE            = 0x02,
    SOMEIP_RC_UNKNOWN_METHOD             = 0x03,
    SOMEIP_RC_NOT_READY                  = 0x04,
    SOMEIP_RC_NOT_REACHABLE              = 0x05,
    SOMEIP_RC_TIMEOUT                    = 0x06,
    SOMEIP_RC_WRONG_PROTOCOL_VERSION     = 0x07,
    SOMEIP_RC_WRONG_INTERFACE_VERSION    = 0x08,
    SOMEIP_RC_MALFORMED_MESSAGE          = 0x09,
    SOMEIP_RC_WRONG_MESSAGE_TYPE         = 0x0A
} SomeIp_ReturnCode_t;

/* -----------------------------------------------------------------------
 * SOME/IP header (AUTOSAR PRS_SOMEIP_00030)
 * ----------------------------------------------------------------------- */
typedef struct {
    uint16_t service_id;        /* Service ID                    */
    uint16_t method_id;         /* Method / Event ID             */
    uint32_t length;            /* Payload length + 8 bytes      */
    uint16_t client_id;         /* Client ID                     */
    uint16_t session_id;        /* Session ID                    */
    uint8_t  protocol_version;  /* Always 0x01                   */
    uint8_t  interface_version; /* Major interface version       */
    uint8_t  message_type;      /* SomeIp_MessageType_t          */
    uint8_t  return_code;       /* SomeIp_ReturnCode_t           */
} SomeIp_Header_t;

/* -----------------------------------------------------------------------
 * SOME/IP message (header + payload pointer)
 * ----------------------------------------------------------------------- */
typedef struct {
    SomeIp_Header_t  header;
    const uint8_t   *payload;
    uint32_t         payload_length;
} SomeIp_Message_t;

/* -----------------------------------------------------------------------
 * Service / event identifiers
 * ----------------------------------------------------------------------- */
typedef uint16_t SomeIp_ServiceId_t;
typedef uint16_t SomeIp_MethodId_t;
typedef uint16_t SomeIp_EventId_t;
typedef uint16_t SomeIp_EventGroupId_t;
typedef uint16_t SomeIp_InstanceId_t;

/* -----------------------------------------------------------------------
 * Std_ReturnType (simplified, matching AUTOSAR Std_Types.h)
 * ----------------------------------------------------------------------- */
#ifndef STD_RETURN_TYPE_DEFINED
#define STD_RETURN_TYPE_DEFINED
typedef uint8_t Std_ReturnType;
#define E_OK     ((Std_ReturnType)0x00u)
#define E_NOT_OK ((Std_ReturnType)0x01u)
#endif

#ifdef __cplusplus
}
#endif

#endif /* SOMEIP_TYPES_H */
