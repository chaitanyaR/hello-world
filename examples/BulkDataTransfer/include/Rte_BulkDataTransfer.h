/**
 * Rte_BulkDataTransfer.h – Generated RTE Header
 * AUTOSAR Release R22-11
 *
 * Generated from: BulkDataTransfer_SWC.arxml
 * DO NOT EDIT MANUALLY – regenerate from ARXML using the RTE generator.
 *
 * Provides the explicit Sender-Receiver API for the BulkDataTransfer SWC:
 *   - Rte_Read_RPort_BulkDataIn_BulkBuffer   (explicit read)
 *   - Rte_Read_RPort_BulkDataIn_Checksum     (explicit read)
 *   - Rte_Write_PPort_BulkDataOut_BulkBuffer (explicit write)
 *   - Rte_Write_PPort_BulkDataOut_Checksum   (explicit write)
 *
 * Port interface: SRIf_BulkData (SENDER-RECEIVER-INTERFACE)
 *   Data elements:
 *     BulkBuffer : BulkDataBufferType  (uint8[256])
 *     Checksum   : BulkDataChecksumType (uint32)
 */

#ifndef RTE_BULKDATATRANSFER_H
#define RTE_BULKDATATRANSFER_H

#include "Std_Types.h"
#include "Compiler.h"

/* =========================================================================
 * Application data types (mirrors ARXML DataTypes package)
 * ====================================================================== */

/** BulkDataBufferType – 256-byte bulk data payload buffer.
 *  Maps to ARRAY-IMPLEMENTATION-DATA-TYPE in ARXML. */
#define BULK_DATA_BUFFER_SIZE   256u
typedef uint8 BulkDataBufferType[BULK_DATA_BUFFER_SIZE];

/** BulkDataChecksumType – CRC-32 checksum of the bulk buffer.
 *  Maps to BulkDataChecksumType IMPLEMENTATION-DATA-TYPE in ARXML. */
typedef uint32 BulkDataChecksumType;

/* =========================================================================
 * RTE return codes  (AUTOSAR_SWS_RTE §7.1.5)
 * ====================================================================== */
typedef uint8 Rte_StatusType;

#define RTE_E_OK           ((Rte_StatusType)0x00u)  /* no error */
#define RTE_E_INVALID      ((Rte_StatusType)0x01u)  /* data not valid */
#define RTE_E_LOST_DATA    ((Rte_StatusType)0x40u)  /* one or more samples lost */
#define RTE_E_NO_DATA      ((Rte_StatusType)0x80u)  /* never written since start */
#define RTE_E_LIMIT        ((Rte_StatusType)0x04u)  /* queue or buffer full */

/* =========================================================================
 * Rte_Instance – instance handle (single-instance SWC, always NULL)
 * ====================================================================== */
typedef void * Rte_Instance;
#define Rte_Self   ((Rte_Instance)0)

/* =========================================================================
 * Explicit Read API
 *   Rte_Read_<Port>_<DataElement>(P2VAR(type,...) data)
 *   Returns RTE_E_OK on success; RTE_E_NO_DATA if never written.
 * ====================================================================== */

/**
 * Rte_Read_RPort_BulkDataIn_BulkBuffer
 * Reads the latest BulkBuffer value from the RPort_BulkDataIn required port.
 *
 * @param[out] data  Pointer to caller-allocated BulkDataBufferType buffer.
 * @return RTE_E_OK      – fresh data copied into *data
 *         RTE_E_NO_DATA – no data written since ECU start
 *         RTE_E_INVALID – alive-timeout exceeded (ALIVE-TIMEOUT = 0.1 s)
 */
FUNC(Rte_StatusType, RTE_CODE)
Rte_Read_RPort_BulkDataIn_BulkBuffer(
    P2VAR(BulkDataBufferType, AUTOMATIC, RTE_APPL_DATA) data);

/**
 * Rte_Read_RPort_BulkDataIn_Checksum
 * Reads the latest Checksum value from the RPort_BulkDataIn required port.
 *
 * @param[out] data  Pointer to caller-allocated BulkDataChecksumType.
 * @return RTE_E_OK      – fresh data copied into *data
 *         RTE_E_NO_DATA – no data written since ECU start
 *         RTE_E_INVALID – alive-timeout exceeded
 */
FUNC(Rte_StatusType, RTE_CODE)
Rte_Read_RPort_BulkDataIn_Checksum(
    P2VAR(BulkDataChecksumType, AUTOMATIC, RTE_APPL_DATA) data);

/* =========================================================================
 * Explicit Write API
 *   Rte_Write_<Port>_<DataElement>(P2CONST(type,...) data)
 *   Returns RTE_E_OK on success.
 * ====================================================================== */

/**
 * Rte_Write_PPort_BulkDataOut_BulkBuffer
 * Writes a BulkBuffer value to the PPort_BulkDataOut provided port.
 *
 * @param[in] data  Pointer to the BulkDataBufferType to send.
 * @return RTE_E_OK – data stored in RTE buffer for transmission
 */
FUNC(Rte_StatusType, RTE_CODE)
Rte_Write_PPort_BulkDataOut_BulkBuffer(
    P2CONST(BulkDataBufferType, AUTOMATIC, RTE_APPL_CONST) data);

/**
 * Rte_Write_PPort_BulkDataOut_Checksum
 * Writes a Checksum value to the PPort_BulkDataOut provided port.
 *
 * @param[in] data  Pointer to the BulkDataChecksumType to send.
 * @return RTE_E_OK – data stored in RTE buffer for transmission
 */
FUNC(Rte_StatusType, RTE_CODE)
Rte_Write_PPort_BulkDataOut_Checksum(
    P2CONST(BulkDataChecksumType, AUTOMATIC, RTE_APPL_CONST) data);

/* =========================================================================
 * RTE lifecycle API
 * ====================================================================== */

/**
 * Rte_Start – initialises the RTE and all configured ports.
 * Must be called once during BSW initialisation, before any task starts.
 */
FUNC(Std_ReturnType, RTE_CODE) Rte_Start(void);

/**
 * Rte_Stop – de-initialises the RTE during shutdown.
 */
FUNC(Std_ReturnType, RTE_CODE) Rte_Stop(void);

/* =========================================================================
 * Simulation helpers – not part of AUTOSAR spec; host build only
 * ====================================================================== */

/**
 * Rte_Sim_InjectBulkBuffer – seeds the R-Port receive buffer with test data.
 * Used by main.c to provide input to the SWC without a real communication stack.
 */
void Rte_Sim_InjectBulkBuffer(const BulkDataBufferType data,
                               BulkDataChecksumType     checksum);

/**
 * Rte_Sim_ReadTxBulkBuffer – reads back the P-Port transmit buffer contents.
 * Used by main.c to verify what the SWC wrote to the output port.
 */
void Rte_Sim_ReadTxBulkBuffer(BulkDataBufferType   out_buffer,
                               BulkDataChecksumType *out_checksum);

#endif /* RTE_BULKDATATRANSFER_H */
