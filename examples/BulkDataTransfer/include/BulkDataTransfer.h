/**
 * BulkDataTransfer.h – SWC Interface Header
 * AUTOSAR Release R22-11
 *
 * Declares the runnable entry-points of the BulkDataTransfer
 * APPLICATION-SW-COMPONENT-TYPE as defined in BulkDataTransfer_SWC.arxml.
 *
 * Runnables:
 *   BulkDataTransfer_InitRunnable  – triggered by INIT-EVENT at ECU start
 *   BulkDataTransfer_10msRunnable  – triggered every 10 ms (TIMING-EVENT)
 */

#ifndef BULKDATATRANSFER_H
#define BULKDATATRANSFER_H

#include "Std_Types.h"
#include "Compiler.h"
#include "Rte_BulkDataTransfer.h"

/* -------------------------------------------------------------------------
 * Module identification
 * ---------------------------------------------------------------------- */
#define BULKDATATRANSFER_VENDOR_ID     0x0000u
#define BULKDATATRANSFER_MODULE_ID     0x0200u
#define BULKDATATRANSFER_INSTANCE_ID   0x00u

/* AUTOSAR software version */
#define BULKDATATRANSFER_SW_MAJOR_VERSION   1u
#define BULKDATATRANSFER_SW_MINOR_VERSION   0u
#define BULKDATATRANSFER_SW_PATCH_VERSION   0u

/* -------------------------------------------------------------------------
 * Runnable declarations
 * ---------------------------------------------------------------------- */

/**
 * BulkDataTransfer_InitRunnable
 *
 * Init runnable – executed once at ECU start (mapped to INIT-EVENT in ARXML).
 * Zeroes the output buffer and writes initial values to the P-Port.
 */
FUNC(void, BULKDATATRANSFER_CODE) BulkDataTransfer_InitRunnable(void);

/**
 * BulkDataTransfer_10msRunnable
 *
 * Cyclic runnable – executed every 10 ms (mapped to TIMING-EVENT_10ms in ARXML).
 * Reads BulkBuffer + Checksum from R-Port, verifies CRC-32, processes the
 * payload, then writes the result to the P-Port.
 */
FUNC(void, BULKDATATRANSFER_CODE) BulkDataTransfer_10msRunnable(void);

#endif /* BULKDATATRANSFER_H */
