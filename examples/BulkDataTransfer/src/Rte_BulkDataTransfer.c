/**
 * Rte_BulkDataTransfer.c – RTE Stub Implementation
 * AUTOSAR Release R22-11
 *
 * Generated from: BulkDataTransfer_SWC.arxml
 * DO NOT EDIT MANUALLY – regenerate from ARXML using the RTE generator.
 *
 * Implements a simple in-process buffer for each data element on the
 * SRIf_BulkData Sender-Receiver interface.  On the host simulation one
 * process acts as both sender and receiver, so the RTE merely copies
 * data between internal buffers.
 *
 * Alive-timeout for R-Port data elements is 100 ms (ALIVE-TIMEOUT = 0.1 s).
 * The stub tracks whether data has been written at least once (NO_DATA guard).
 */

#include "Rte_BulkDataTransfer.h"
#include "Std_Types.h"
#include "Compiler.h"

#include <string.h>  /* memcpy, memset */

/* =========================================================================
 * Internal buffer state
 * ====================================================================== */

typedef struct
{
    BulkDataBufferType   Buffer;
    boolean              DataValid;  /* TRUE after first write */
} Rte_BulkBufferSlot;

typedef struct
{
    BulkDataChecksumType Checksum;
    boolean              DataValid;
} Rte_ChecksumSlot;

/* R-Port side – written by a simulated upstream sender or by the SWC test */
static Rte_BulkBufferSlot  Rte_RxBulkBuffer;
static Rte_ChecksumSlot    Rte_RxChecksum;

/* P-Port side – written by the SWC, readable by a simulated downstream */
static Rte_BulkBufferSlot  Rte_TxBulkBuffer;
static Rte_ChecksumSlot    Rte_TxChecksum;

static boolean Rte_Initialised = FALSE;

/* =========================================================================
 * Rte_Start / Rte_Stop
 * ====================================================================== */

FUNC(Std_ReturnType, RTE_CODE) Rte_Start(void)
{
    (void)memset(&Rte_RxBulkBuffer, 0, sizeof(Rte_RxBulkBuffer));
    (void)memset(&Rte_RxChecksum,   0, sizeof(Rte_RxChecksum));
    (void)memset(&Rte_TxBulkBuffer, 0, sizeof(Rte_TxBulkBuffer));
    (void)memset(&Rte_TxChecksum,   0, sizeof(Rte_TxChecksum));

    Rte_Initialised = TRUE;
    return E_OK;
}

FUNC(Std_ReturnType, RTE_CODE) Rte_Stop(void)
{
    Rte_Initialised = FALSE;
    return E_OK;
}

/* =========================================================================
 * Explicit Read – RPort_BulkDataIn
 * ====================================================================== */

FUNC(Rte_StatusType, RTE_CODE)
Rte_Read_RPort_BulkDataIn_BulkBuffer(
    P2VAR(BulkDataBufferType, AUTOMATIC, RTE_APPL_DATA) data)
{
    if (data == NULL_PTR)
    {
        return RTE_E_INVALID;
    }

    if (Rte_RxBulkBuffer.DataValid == FALSE)
    {
        /* Return zero-initialised data and signal NO_DATA */
        (void)memset(*data, 0, sizeof(BulkDataBufferType));
        return RTE_E_NO_DATA;
    }

    (void)memcpy(*data, Rte_RxBulkBuffer.Buffer, sizeof(BulkDataBufferType));
    return RTE_E_OK;
}

FUNC(Rte_StatusType, RTE_CODE)
Rte_Read_RPort_BulkDataIn_Checksum(
    P2VAR(BulkDataChecksumType, AUTOMATIC, RTE_APPL_DATA) data)
{
    if (data == NULL_PTR)
    {
        return RTE_E_INVALID;
    }

    if (Rte_RxChecksum.DataValid == FALSE)
    {
        *data = 0u;
        return RTE_E_NO_DATA;
    }

    *data = Rte_RxChecksum.Checksum;
    return RTE_E_OK;
}

/* =========================================================================
 * Explicit Write – PPort_BulkDataOut
 * ====================================================================== */

FUNC(Rte_StatusType, RTE_CODE)
Rte_Write_PPort_BulkDataOut_BulkBuffer(
    P2CONST(BulkDataBufferType, AUTOMATIC, RTE_APPL_CONST) data)
{
    if (data == NULL_PTR)
    {
        return RTE_E_INVALID;
    }

    (void)memcpy(Rte_TxBulkBuffer.Buffer, *data, sizeof(BulkDataBufferType));
    Rte_TxBulkBuffer.DataValid = TRUE;
    return RTE_E_OK;
}

FUNC(Rte_StatusType, RTE_CODE)
Rte_Write_PPort_BulkDataOut_Checksum(
    P2CONST(BulkDataChecksumType, AUTOMATIC, RTE_APPL_CONST) data)
{
    if (data == NULL_PTR)
    {
        return RTE_E_INVALID;
    }

    Rte_TxChecksum.Checksum   = *data;
    Rte_TxChecksum.DataValid  = TRUE;
    return RTE_E_OK;
}

/* =========================================================================
 * Test / simulation helper – inject data into the R-Port receive buffer.
 * Not part of the AUTOSAR spec; used by main.c to seed input data.
 * ====================================================================== */

void Rte_Sim_InjectBulkBuffer(const BulkDataBufferType data,
                               BulkDataChecksumType     checksum)
{
    (void)memcpy(Rte_RxBulkBuffer.Buffer, data, sizeof(BulkDataBufferType));
    Rte_RxBulkBuffer.DataValid = TRUE;

    Rte_RxChecksum.Checksum   = checksum;
    Rte_RxChecksum.DataValid  = TRUE;
}

void Rte_Sim_ReadTxBulkBuffer(BulkDataBufferType   out_buffer,
                               BulkDataChecksumType *out_checksum)
{
    if (out_buffer != NULL_PTR)
    {
        (void)memcpy(out_buffer, Rte_TxBulkBuffer.Buffer,
                     sizeof(BulkDataBufferType));
    }
    if (out_checksum != NULL_PTR)
    {
        *out_checksum = Rte_TxChecksum.Checksum;
    }
}
