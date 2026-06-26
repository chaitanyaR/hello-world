/**
 * ComStack_Types.h – AUTOSAR Communication Stack Types
 * AUTOSAR Release 22-11  |  AUTOSAR_SWS_CommunicationStackTypes
 *
 * Provides types shared across the AUTOSAR communication stack (COM, PduR,
 * SoAd, SomeIp, SomeIpSd, etc.).  In a full AUTOSAR project this header is
 * provided by the BSW integration package.
 */

#ifndef COMSTACK_TYPES_H
#define COMSTACK_TYPES_H

#include "Std_Types.h"

/* ----- Vendor / Module identification ------------------------------------ */
#define COMSTACK_VENDOR_ID          0x0000u
#define COMSTACK_MODULE_ID          0x00B9u

/* ----- AUTOSAR version --------------------------------------------------- */
#define COMSTACK_AR_RELEASE_MAJOR_VERSION   22u
#define COMSTACK_AR_RELEASE_MINOR_VERSION   11u
#define COMSTACK_AR_RELEASE_PATCH_VERSION    0u

/* ----- PDU types --------------------------------------------------------- */

/** Identifies a PDU within a module (handle ID). */
typedef uint16 PduIdType;

/** Length of a PDU payload in bytes. */
typedef uint32 PduLengthType;

/**
 * Carries a PDU data pointer and its length.
 * MetaDataPtr – pointer to meta-data associated with the PDU (NULL if none).
 */
typedef struct {
    uint8*        SduDataPtr;
    uint8*        MetaDataPtr;
    PduLengthType SduLength;
} PduInfoType;

/** Result of a network buffer request. */
typedef enum {
    BUFREQ_OK       = 0x00u,
    BUFREQ_E_NOT_OK = 0x01u,
    BUFREQ_E_BUSY   = 0x02u,
    BUFREQ_E_OVFL   = 0x03u
} BufReq_ReturnType;

/** Transmission mode (used by lower-layer drivers). */
typedef enum {
    TP_DATACONF     = 0x00u,
    TP_DATARETRY    = 0x01u,
    TP_CONFPENDING  = 0x02u
} TpDataStateType;

/** Network handle for SoAd / TcpIp integration. */
typedef uint16 SoAd_SoConIdType;
typedef uint16 SoAd_RoutingGroupIdType;

#endif /* COMSTACK_TYPES_H */
