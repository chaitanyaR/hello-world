/**
 * BulkDataTransfer.c – SWC Implementation
 * AUTOSAR Release R22-11
 *
 * Implements the BulkDataTransfer APPLICATION-SW-COMPONENT-TYPE
 * as modelled in BulkDataTransfer_SWC.arxml.
 *
 * Runnables
 * ---------
 *  BulkDataTransfer_InitRunnable
 *    Triggered by: INIT-EVENT (once at ECU start)
 *    Action:       Zeroes the output buffer and writes initial values to the
 *                  PPort via explicit RTE write calls.
 *
 *  BulkDataTransfer_10msRunnable
 *    Triggered by: TIMING-EVENT (every 10 ms, via OsAlarm_10ms)
 *    Action:       Explicit-reads BulkBuffer + Checksum from RPort_BulkDataIn.
 *                  Verifies the CRC-32 of the received payload.
 *                  Transforms the data (byte-wise inversion of each element).
 *                  Computes a new CRC-32 for the output.
 *                  Explicit-writes the result to PPort_BulkDataOut.
 *
 * CRC-32
 * ------
 * Uses the ISO 3309 / ITU-T V.42 polynomial (0xEDB88320 reflected),
 * computed via a simple bit-loop suitable for embedded targets.
 * A dedicated CRC BSW module would replace this in a full AUTOSAR stack.
 */

#include "BulkDataTransfer.h"
#include "Rte_BulkDataTransfer.h"
#include "Std_Types.h"
#include "Compiler.h"

#include <string.h>   /* memset, memcpy */
#include <stdio.h>    /* printf – host simulation logging only */

/* =========================================================================
 * Private: CRC-32 (ISO 3309, reflected polynomial)
 * ====================================================================== */

#define CRC32_INIT_VALUE   0xFFFFFFFFUL
#define CRC32_FINAL_XOR    0xFFFFFFFFUL
#define CRC32_POLYNOMIAL   0xEDB88320UL

static BulkDataChecksumType BulkDataTransfer_Crc32(
    const uint8  *data,
    uint32        length)
{
    uint32 crc = CRC32_INIT_VALUE;
    uint32 i;
    uint8  bit;

    for (i = 0u; i < length; i++)
    {
        crc ^= (uint32)data[i];
        for (bit = 0u; bit < 8u; bit++)
        {
            if ((crc & 0x00000001UL) != 0u)
            {
                crc = (crc >> 1u) ^ CRC32_POLYNOMIAL;
            }
            else
            {
                crc >>= 1u;
            }
        }
    }

    return (BulkDataChecksumType)(crc ^ CRC32_FINAL_XOR);
}

/* =========================================================================
 * BulkDataTransfer_InitRunnable
 * INIT-EVENT – executed once by the RTE after Rte_Start()
 * ====================================================================== */

FUNC(void, BULKDATATRANSFER_CODE) BulkDataTransfer_InitRunnable(void)
{
    BulkDataBufferType   initBuffer;
    BulkDataChecksumType initChecksum;
    Rte_StatusType       rteStatus;

    /* Zero-initialise the output buffer */
    (void)memset(initBuffer, 0, sizeof(BulkDataBufferType));
    initChecksum = BulkDataTransfer_Crc32(initBuffer, BULK_DATA_BUFFER_SIZE);

    /* Write initial values to the provided port (explicit send) */
    rteStatus = Rte_Write_PPort_BulkDataOut_BulkBuffer(
                    (P2CONST(BulkDataBufferType, AUTOMATIC, RTE_APPL_CONST))&initBuffer);
    if (rteStatus != RTE_E_OK)
    {
        /* Host simulation: log and continue; on target call DET or error handler */
        printf("[BulkDataTransfer] InitRunnable: write BulkBuffer failed (0x%02X)\n",
               (unsigned)rteStatus);
    }

    rteStatus = Rte_Write_PPort_BulkDataOut_Checksum(&initChecksum);
    if (rteStatus != RTE_E_OK)
    {
        printf("[BulkDataTransfer] InitRunnable: write Checksum failed (0x%02X)\n",
               (unsigned)rteStatus);
    }

    printf("[BulkDataTransfer] InitRunnable: output buffer zeroed, CRC=0x%08X\n",
           (unsigned)initChecksum);
}

/* =========================================================================
 * BulkDataTransfer_10msRunnable
 * TIMING-EVENT – executed every 10 ms by OsTask_10ms
 * ====================================================================== */

FUNC(void, BULKDATATRANSFER_CODE) BulkDataTransfer_10msRunnable(void)
{
    BulkDataBufferType   rxBuffer;
    BulkDataChecksumType rxChecksum;
    BulkDataBufferType   txBuffer;
    BulkDataChecksumType txChecksum;
    BulkDataChecksumType computedCrc;
    Rte_StatusType       rteStatus;
    uint32               i;

    /* ------------------------------------------------------------------
     * Step 1: Explicit read from R-Port
     * ------------------------------------------------------------------ */
    rteStatus = Rte_Read_RPort_BulkDataIn_BulkBuffer(
                    (P2VAR(BulkDataBufferType, AUTOMATIC, RTE_APPL_DATA))&rxBuffer);

    if (rteStatus == RTE_E_NO_DATA)
    {
        /* No data available yet – skip this cycle */
        return;
    }
    if (rteStatus != RTE_E_OK)
    {
        printf("[BulkDataTransfer] 10msRunnable: read BulkBuffer error (0x%02X)\n",
               (unsigned)rteStatus);
        return;
    }

    rteStatus = Rte_Read_RPort_BulkDataIn_Checksum(&rxChecksum);
    if (rteStatus != RTE_E_OK)
    {
        printf("[BulkDataTransfer] 10msRunnable: read Checksum error (0x%02X)\n",
               (unsigned)rteStatus);
        return;
    }

    /* ------------------------------------------------------------------
     * Step 2: CRC-32 verification of received payload
     * ------------------------------------------------------------------ */
    computedCrc = BulkDataTransfer_Crc32(rxBuffer, BULK_DATA_BUFFER_SIZE);

    if (computedCrc != rxChecksum)
    {
        printf("[BulkDataTransfer] 10msRunnable: CRC mismatch "
               "expected=0x%08X got=0x%08X – discarding frame\n",
               (unsigned)computedCrc, (unsigned)rxChecksum);
        return;
    }

    /* ------------------------------------------------------------------
     * Step 3: Process payload
     * Transformation: bitwise inversion of every byte.
     * This represents a minimal processing step; replace with application
     * logic (e.g., SOME/IP payload forwarding, signal extraction, etc.).
     * ------------------------------------------------------------------ */
    for (i = 0u; i < BULK_DATA_BUFFER_SIZE; i++)
    {
        txBuffer[i] = (uint8)(~rxBuffer[i]);
    }

    /* ------------------------------------------------------------------
     * Step 4: Compute CRC-32 for the output payload
     * ------------------------------------------------------------------ */
    txChecksum = BulkDataTransfer_Crc32(txBuffer, BULK_DATA_BUFFER_SIZE);

    /* ------------------------------------------------------------------
     * Step 5: Explicit write to P-Port
     * ------------------------------------------------------------------ */
    rteStatus = Rte_Write_PPort_BulkDataOut_BulkBuffer(
                    (P2CONST(BulkDataBufferType, AUTOMATIC, RTE_APPL_CONST))&txBuffer);
    if (rteStatus != RTE_E_OK)
    {
        printf("[BulkDataTransfer] 10msRunnable: write BulkBuffer error (0x%02X)\n",
               (unsigned)rteStatus);
        return;
    }

    rteStatus = Rte_Write_PPort_BulkDataOut_Checksum(&txChecksum);
    if (rteStatus != RTE_E_OK)
    {
        printf("[BulkDataTransfer] 10msRunnable: write Checksum error (0x%02X)\n",
               (unsigned)rteStatus);
        return;
    }

    printf("[BulkDataTransfer] 10msRunnable: OK "
           "rxCRC=0x%08X txCRC=0x%08X\n",
           (unsigned)rxChecksum, (unsigned)txChecksum);
}
