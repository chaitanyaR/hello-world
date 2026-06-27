/**
 * Os_Cfg.h – Generated OS Configuration Header
 * AUTOSAR Release R22-11
 *
 * Generated from: OsConfig.arxml
 * DO NOT EDIT MANUALLY – regenerate from ARXML using the OS configuration tool.
 *
 * Declares application modes, task IDs, alarm IDs, and counter IDs so that
 * BSW modules and the application can reference them by symbolic name.
 */

#ifndef OS_CFG_H
#define OS_CFG_H

#include "Os.h"

/* -------------------------------------------------------------------------
 * Application Modes
 * ---------------------------------------------------------------------- */
/** Default application mode (OSEK mandated, always index 0). */
#define OSDEFAULTAPPMODE   ((AppModeType)0x00u)

/* -------------------------------------------------------------------------
 * Task identifiers
 * Assigned sequentially; order must match Os_Cfg.c task table.
 * ---------------------------------------------------------------------- */
#define OsTask_10ms   ((TaskType)0x00u)

/** Total number of configured tasks. */
#define OS_NUM_TASKS   1u

/* -------------------------------------------------------------------------
 * Counter identifiers
 * ---------------------------------------------------------------------- */
#define OsCounter_SystemTimer   ((uint8)0x00u)

/** Total number of configured counters. */
#define OS_NUM_COUNTERS   1u

/* -------------------------------------------------------------------------
 * Alarm identifiers
 * ---------------------------------------------------------------------- */
#define OsAlarm_10ms   ((AlarmType)0x00u)

/** Total number of configured alarms. */
#define OS_NUM_ALARMS   1u

/* -------------------------------------------------------------------------
 * Task body forward declarations
 * ---------------------------------------------------------------------- */
DeclareTask(OsTask_10ms);

/* -------------------------------------------------------------------------
 * Alarm configuration constants (mirror of ARXML values)
 * ---------------------------------------------------------------------- */
/** Counter ticks per millisecond (1 tick = 1 ms). */
#define OS_TICKS_PER_MS   1u

/** OsAlarm_10ms: period in counter ticks. */
#define OSALARM_10MS_CYCLE   ((TickType)10u)

/** OsAlarm_10ms: initial offset in counter ticks. */
#define OSALARM_10MS_OFFSET  ((TickType)5u)

/* -------------------------------------------------------------------------
 * Stack sizes (bytes) – mirrors ARXML OsTaskStackSize
 * ---------------------------------------------------------------------- */
#define OSTASK_10MS_STACK_SIZE   512u

#endif /* OS_CFG_H */
