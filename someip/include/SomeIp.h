/**
 * SomeIp.h – SOME/IP Transformer Module API
 * AUTOSAR Release 22-11
 * Reference: AUTOSAR_SWS_SOMEIPTransformer R22-11
 *
 * Provides the public API of the SOME/IP transformer module:
 *   - Lifecycle   : SomeIp_Init, SomeIp_GetVersionInfo, SomeIp_MainFunction
 *   - Transformer : SomeIp_Serialize, SomeIp_Deserialize, SomeIp_ValidateHeader
 *   - Indication  : SomeIp_RxIndication, SomeIp_TxConfirmation
 */

#ifndef SOMEIP_H
#define SOMEIP_H

/* ----- Includes ---------------------------------------------------------- */
#include "SomeIp_Types.h"
#include "SomeIp_Cfg.h"

/* ----- Vendor / Module identification ------------------------------------ */
#define SOMEIP_VENDOR_ID        0x0000u
#define SOMEIP_MODULE_ID        0x00A9u     /**< AUTOSAR BSW Module ID 169 */
#define SOMEIP_INSTANCE_ID      0x00u

/* ----- AUTOSAR release version ------------------------------------------- */
#define SOMEIP_AR_RELEASE_MAJOR_VERSION     22u
#define SOMEIP_AR_RELEASE_MINOR_VERSION     11u
#define SOMEIP_AR_RELEASE_PATCH_VERSION      0u

/* ----- Software version -------------------------------------------------- */
#define SOMEIP_SW_MAJOR_VERSION     1u
#define SOMEIP_SW_MINOR_VERSION     0u
#define SOMEIP_SW_PATCH_VERSION     0u

/* ----- Version consistency checks (SWS_BSW_00004) ----------------------- */
#if (SOMEIP_TYPES_AR_RELEASE_MAJOR_VERSION != SOMEIP_AR_RELEASE_MAJOR_VERSION)
#  error "SomeIp_Types.h: AR major version mismatch"
#endif
#if (SOMEIP_CFG_AR_RELEASE_MAJOR_VERSION != SOMEIP_AR_RELEASE_MAJOR_VERSION)
#  error "SomeIp_Cfg.h: AR major version mismatch"
#endif

/* =========================================================================
 * Development Error IDs  (SWS_SomeIp_00xxx)
 * Reported via Det_ReportError(SOMEIP_MODULE_ID, SOMEIP_INSTANCE_ID, ApiId, ErrId)
 * ========================================================================= */
#define SOMEIP_E_NULL_PTR           0x01u   /**< NULL pointer argument       */
#define SOMEIP_E_INV_ARG            0x02u   /**< Invalid argument value      */
#define SOMEIP_E_UNINIT             0x03u   /**< Module not initialised      */
#define SOMEIP_E_BUFF_TOO_SMALL     0x04u   /**< Output buffer too small     */
#define SOMEIP_E_INV_PDU_SDU_ID     0x05u   /**< Unknown PDU/SDU identifier  */

/* =========================================================================
 * API Service IDs (used as ApiId in Det_ReportError)
 * ========================================================================= */
#define SOMEIP_SID_INIT                 0x01u
#define SOMEIP_SID_GET_VERSION_INFO     0x02u
#define SOMEIP_SID_MAIN_FUNCTION        0x03u
#define SOMEIP_SID_RX_INDICATION        0x10u
#define SOMEIP_SID_TX_CONFIRMATION      0x11u
#define SOMEIP_SID_SERIALIZE            0x20u
#define SOMEIP_SID_DESERIALIZE          0x21u
#define SOMEIP_SID_VALIDATE_HEADER      0x22u

/* =========================================================================
 * Public API
 * ========================================================================= */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SomeIp_Init
 * Initialises the SOME/IP transformer module.
 * Must be called before any other SomeIp_* function.
 *
 * @param ConfigPtr  Pointer to the module configuration.
 *                   NULL uses compile-time defaults (allowed per AUTOSAR).
 */
void SomeIp_Init(const SomeIp_ConfigType* ConfigPtr);

/**
 * SomeIp_GetVersionInfo
 * Returns the version information of the SOME/IP module.
 * Only available when SOMEIP_VERSION_INFO_API == STD_ON.
 *
 * @param VersioninfoPtr  Output: version info structure (must not be NULL).
 */
#if (SOMEIP_VERSION_INFO_API == STD_ON)
void SomeIp_GetVersionInfo(Std_VersionInfoType* VersioninfoPtr);
#endif

/**
 * SomeIp_MainFunction
 * Scheduled cyclic function (Timing: system integrator configurable).
 * Handles session-ID housekeeping and deferred operations.
 */
void SomeIp_MainFunction(void);

/**
 * SomeIp_RxIndication
 * Called by SoAd when a SOME/IP frame is received from the network.
 *
 * @param RxPduId     Receive PDU identifier (from PduR routing table).
 * @param PduInfoPtr  PDU data pointer + length.
 */
void SomeIp_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr);

/**
 * SomeIp_TxConfirmation
 * Called by SoAd after a SOME/IP frame has been successfully transmitted.
 *
 * @param TxPduId  Transmit PDU identifier.
 * @param Result   E_OK: transmitted; E_NOT_OK: failed.
 */
void SomeIp_TxConfirmation(PduIdType TxPduId, Std_ReturnType Result);

/* =========================================================================
 * Serializer / Deserializer API
 * ========================================================================= */

/**
 * SomeIp_Serialize
 * Encodes a SOME/IP message (header + payload) into a contiguous wire-format
 * byte buffer in big-endian byte order (PRS_SOMEIP_00030).
 *
 * @param HeaderPtr   [in]  Header fields to encode.
 * @param PayloadPtr  [in]  PDU carrying payload data (NULL → no payload).
 * @param BufPtr      [out] Destination buffer for the encoded frame.
 * @param BufLenPtr   [in/out] In: size of BufPtr. Out: bytes written on E_OK.
 * @return E_OK on success, E_NOT_OK on any error.
 *
 * DET errors: SOMEIP_E_UNINIT, SOMEIP_E_NULL_PTR, SOMEIP_E_BUFF_TOO_SMALL.
 */
Std_ReturnType SomeIp_Serialize(
    const SomeIp_HeaderType* HeaderPtr,
    const PduInfoType*       PayloadPtr,
    uint8*                   BufPtr,
    uint32*                  BufLenPtr);

/**
 * SomeIp_Deserialize
 * Parses a raw wire-format byte buffer into a SOME/IP header and payload
 * reference. Zero-copy: PayloadPtr->SduDataPtr points into BufPtr.
 *
 * @param BufPtr      [in]  Raw SOME/IP frame.
 * @param BufLen      [in]  Frame length in bytes.
 * @param HeaderPtr   [out] Parsed header fields.
 * @param PayloadPtr  [out] Payload pointer (into BufPtr) and length.
 * @return E_OK on success, E_NOT_OK on any error.
 *
 * DET errors: SOMEIP_E_UNINIT, SOMEIP_E_NULL_PTR, SOMEIP_E_INV_ARG.
 */
Std_ReturnType SomeIp_Deserialize(
    const uint8*       BufPtr,
    uint32             BufLen,
    SomeIp_HeaderType* HeaderPtr,
    PduInfoType*       PayloadPtr);

/**
 * SomeIp_ValidateHeader
 * Checks all mandatory header constraints defined in PRS_SOMEIP_00052 and
 * PRS_SOMEIP_00055.
 *
 * @param HeaderPtr  [in] Header to validate.
 * @return E_OK if all constraints pass, E_NOT_OK otherwise.
 *
 * DET errors: SOMEIP_E_UNINIT, SOMEIP_E_NULL_PTR.
 */
Std_ReturnType SomeIp_ValidateHeader(const SomeIp_HeaderType* HeaderPtr);

#ifdef __cplusplus
}
#endif

#endif /* SOMEIP_H */
