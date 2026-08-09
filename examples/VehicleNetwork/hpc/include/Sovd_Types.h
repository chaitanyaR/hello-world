/**
 * Sovd_Types.h  –  ASAM SOVD 1.0 / AUTOSAR R22-11 Type Definitions
 *
 * Implements data types aligned with:
 *   ASAM SOVD 1.0.0  (Service-Oriented Vehicle Diagnostics)
 *   ISO 14229-1:2020  (UDS – Unified Diagnostic Services)
 *   AUTOSAR_SWS_DiagnosticCommunicationManager R22-11
 *
 * SOVD replaces classic DoIP/UDS-over-CAN with HTTP/REST + JSON, allowing
 * diagnostic clients (testers, cloud backends, fleet tools) to communicate
 * with vehicle ECUs via standard web technology while retaining ISO 14229
 * fault code semantics.
 */

#ifndef SOVD_TYPES_H
#define SOVD_TYPES_H

#include "Std_Types.h"   /* uint8, uint16, uint32, sint8, float32, boolean */

/* =========================================================================
 * SOVD return / status codes (ASAM SOVD 1.0 §6.3)
 * ====================================================================== */
typedef uint8 Sovd_StatusType;

#define SOVD_STATUS_OK                  ((Sovd_StatusType)0x00u)
#define SOVD_STATUS_GENERAL_REJECT      ((Sovd_StatusType)0x10u)
#define SOVD_STATUS_CONDITIONS_NOT_MET  ((Sovd_StatusType)0x22u)
#define SOVD_STATUS_REQUEST_OUT_OF_RANGE ((Sovd_StatusType)0x31u)
#define SOVD_STATUS_SECURITY_ACCESS_DENIED ((Sovd_StatusType)0x33u)
#define SOVD_STATUS_NOT_FOUND           ((Sovd_StatusType)0x7Eu)
#define SOVD_STATUS_NOT_SUPPORTED       ((Sovd_StatusType)0x7Fu)

/* =========================================================================
 * Data element access categories (ASAM SOVD 1.0 §7.2)
 * ====================================================================== */
typedef uint8 Sovd_AccessType;

#define SOVD_ACCESS_READ                ((Sovd_AccessType)0x01u)
#define SOVD_ACCESS_WRITE               ((Sovd_AccessType)0x02u)
#define SOVD_ACCESS_READ_WRITE          ((Sovd_AccessType)0x03u)

/* =========================================================================
 * Data element categories (ASAM SOVD 1.0 §7.2.1)
 * ====================================================================== */
typedef uint8 Sovd_DataCategoryType;

#define SOVD_CATEGORY_CURRENT           ((Sovd_DataCategoryType)0x01u) /* live ECU values */
#define SOVD_CATEGORY_IDENTIFICATION    ((Sovd_DataCategoryType)0x02u) /* ECU ID / VIN     */
#define SOVD_CATEGORY_CONFIGURATION     ((Sovd_DataCategoryType)0x04u) /* calibration data */
#define SOVD_CATEGORY_STORED            ((Sovd_DataCategoryType)0x08u) /* freeze-frame     */

/* =========================================================================
 * Primitive data type identifiers (mapped from ISO 14229-1 §A.1)
 * ====================================================================== */
typedef uint8 Sovd_DataTypeId;

#define SOVD_DTYPE_UINT8                ((Sovd_DataTypeId)0x01u)
#define SOVD_DTYPE_UINT16               ((Sovd_DataTypeId)0x02u)
#define SOVD_DTYPE_UINT32               ((Sovd_DataTypeId)0x03u)
#define SOVD_DTYPE_SINT8                ((Sovd_DataTypeId)0x04u)
#define SOVD_DTYPE_SINT16               ((Sovd_DataTypeId)0x05u)
#define SOVD_DTYPE_FLOAT32              ((Sovd_DataTypeId)0x07u)
#define SOVD_DTYPE_BOOLEAN              ((Sovd_DataTypeId)0x0Au)
#define SOVD_DTYPE_STRING               ((Sovd_DataTypeId)0x0Bu)

/* =========================================================================
 * DTC status byte bit masks (ISO 14229-1:2020 §D.2 – DTC Status Mask)
 * ====================================================================== */
#define SOVD_DTC_TEST_FAILED            ((uint8)0x01u)  /* bit 0 */
#define SOVD_DTC_FAILED_THIS_CYCLE      ((uint8)0x02u)  /* bit 1 */
#define SOVD_DTC_PENDING                ((uint8)0x04u)  /* bit 2 */
#define SOVD_DTC_CONFIRMED              ((uint8)0x08u)  /* bit 3 */
#define SOVD_DTC_NOT_COMPLETED_CLEAR    ((uint8)0x10u)  /* bit 4 */
#define SOVD_DTC_FAILED_SINCE_CLEAR     ((uint8)0x20u)  /* bit 5 */
#define SOVD_DTC_NOT_COMPLETED_CYCLE    ((uint8)0x40u)  /* bit 6 */
#define SOVD_DTC_WARNING_INDICATOR      ((uint8)0x80u)  /* bit 7 */

/* Composite status helpers */
#define SOVD_DTC_STATUS_CONFIRMED       (SOVD_DTC_TEST_FAILED | SOVD_DTC_CONFIRMED | SOVD_DTC_FAILED_SINCE_CLEAR)  /* 0x29 */
#define SOVD_DTC_STATUS_PENDING         (SOVD_DTC_PENDING | SOVD_DTC_NOT_COMPLETED_CLEAR)  /* 0x14 */

/* =========================================================================
 * DTC severity (ASAM SOVD 1.0 maps to ISO 14229-1 §D.4)
 * ====================================================================== */
typedef uint8 Sovd_DtcSeverityType;

#define SOVD_DTC_SEV_NO_SEV             ((Sovd_DtcSeverityType)0x00u)
#define SOVD_DTC_SEV_MAINTENANCE_ONLY   ((Sovd_DtcSeverityType)0x20u)
#define SOVD_DTC_SEV_CHECK_AT_NEXT_HALT ((Sovd_DtcSeverityType)0x40u)
#define SOVD_DTC_SEV_CHECK_IMMEDIATELY  ((Sovd_DtcSeverityType)0x80u)

/* =========================================================================
 * Fault record (SOVD fault resource entry, ASAM SOVD 1.0 §9.4)
 * ====================================================================== */
typedef struct {
    uint32               dtcCode;       /* 3-byte OBD DTC in lower 24 bits   */
    uint8                statusByte;    /* ISO 14229-1 DTC status mask        */
    Sovd_DtcSeverityType severity;
    uint8                active;        /* 1 = present, 0 = cleared          */
    const char          *description;  /* human-readable string (ROM ptr)    */
} Sovd_FaultRecordType;

/* =========================================================================
 * SOVD HTTP server configuration
 * ====================================================================== */
typedef struct {
    uint16 httpPort;       /* TCP port for SOVD REST interface (default 8080) */
    uint8  maxPendingConn; /* listen() backlog                                 */
} Sovd_ServerConfigType;

/* =========================================================================
 * Component identifiers (ASAM SOVD 1.0 §8 – component URI path segment)
 * ====================================================================== */
#define SOVD_COMP_ADAS      "adas"
#define SOVD_COMP_ECM       "ecm"
#define SOVD_COMP_TCU       "tcu"
#define SOVD_COMP_BCM       "bcm"
#define SOVD_COMP_GATEWAY   "gateway"

/* Maximum simultaneous SOVD component entries */
#define SOVD_MAX_FAULTS_PER_COMP  ((uint8)8u)

#endif /* SOVD_TYPES_H */
