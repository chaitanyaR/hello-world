/**
 * Os_Cfg.c – Generated OS Task / Alarm Definitions
 * AUTOSAR Release R22-11
 *
 * Generated from: OsConfig.arxml
 * DO NOT EDIT MANUALLY – regenerate from ARXML using the OS configuration tool.
 *
 * Defines:
 *  TASK(OsTask_10ms) – the 10 ms cyclic task body that calls the
 *                      BulkDataTransfer_10msRunnable() runnable.
 *
 * The OS stub in Os.c discovers all task bodies via the function-pointer
 * table Os_TaskTable[] declared at the bottom of this file.
 */

#include "Os.h"
#include "Os_Cfg.h"
#include "BulkDataTransfer.h"

#include <stdio.h>   /* printf – host simulation logging only */

/* =========================================================================
 * Task body: OsTask_10ms
 *
 * Activated every 10 ms by OsAlarm_10ms (configured in OsConfig.arxml).
 * Priority : 5  (FULL preemptive scheduling)
 * Stack    : 512 bytes
 *
 * Calls the BulkDataTransfer_10msRunnable as specified by the
 * TIMING-EVENT → RUNNABLE mapping in BulkDataTransfer_SWC.arxml.
 * ====================================================================== */

TASK(OsTask_10ms)
{
    /* Call all runnables mapped to this task in the OsTask_10ms period */
    BulkDataTransfer_10msRunnable();

    /* Every basic task must terminate itself */
    (void)TerminateTask();
}

/* =========================================================================
 * Task dispatch table used by the OS stub scheduler.
 * Order must match task identifier values in Os_Cfg.h.
 * ====================================================================== */

typedef void (*OsTaskBodyFnPtr)(void);

const OsTaskBodyFnPtr Os_TaskTable[OS_NUM_TASKS] =
{
    OsTask_10ms_taskbody   /* index 0 = OsTask_10ms */
};
