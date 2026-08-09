/**
 * Os.c – AUTOSAR OS Stub for Host Simulation
 * AUTOSAR Release R22-11
 *
 * Implements the OSEK/AUTOSAR OS API for the host-simulation build.
 * On a real target this would be the RTOS kernel; here it is a simple
 * round-robin scheduler that runs a fixed number of 10 ms cycles.
 *
 * The task table Os_TaskTable[] is populated by Os_Cfg.c and maps each
 * TaskType identifier to its generated task body function pointer.
 */

#include "Os.h"
#include "Os_Cfg.h"

#include <stdio.h>
#include <stdlib.h>

/* =========================================================================
 * External references
 * ====================================================================== */

/** Task body dispatch table defined in Os_Cfg.c */
typedef void (*OsTaskBodyFnPtr)(void);
extern const OsTaskBodyFnPtr Os_TaskTable[OS_NUM_TASKS];

/* =========================================================================
 * Internal state
 * ====================================================================== */

/** Number of simulated 10 ms scheduler ticks to execute. */
#define OS_SIM_TICKS   10u

static volatile boolean Os_Running    = FALSE;
static TaskType         Os_CurrentTask = (TaskType)0xFFu;

/* =========================================================================
 * Hook weak stubs (application overrides these in main.c)
 * ====================================================================== */

__attribute__((weak)) void StartupHook(void)  { (void)0; }
__attribute__((weak)) void ShutdownHook(StatusType e) { (void)e; }
__attribute__((weak)) void ErrorHook(StatusType e)    { (void)e; }
__attribute__((weak)) void PreTaskHook(void)  { (void)0; }
__attribute__((weak)) void PostTaskHook(void) { (void)0; }

/* =========================================================================
 * StartOS
 * ====================================================================== */

FUNC(void, OS_CODE) StartOS(AppModeType Mode)
{
    uint32 tick;
    (void)Mode;

    Os_Running = TRUE;
    StartupHook();

    /* Simulate OS_SIM_TICKS × 10 ms scheduler ticks */
    for (tick = 0u; tick < OS_SIM_TICKS; tick++)
    {
        /* OsAlarm_10ms fires every tick and activates OsTask_10ms */
        Os_CurrentTask = OsTask_10ms;

        PreTaskHook();
        Os_TaskTable[OsTask_10ms]();   /* invoke task body */
        PostTaskHook();

        Os_CurrentTask = (TaskType)0xFFu;
    }

    Os_Running = FALSE;
}

/* =========================================================================
 * ShutdownOS
 * ====================================================================== */

FUNC(void, OS_CODE) ShutdownOS(StatusType Error)
{
    ShutdownHook(Error);
    Os_Running = FALSE;
}

/* =========================================================================
 * TerminateTask
 * ====================================================================== */

FUNC(StatusType, OS_CODE) TerminateTask(void)
{
    /* On host: nothing to do – task body simply returns */
    return (StatusType)E_OK;
}

/* =========================================================================
 * ActivateTask
 * ====================================================================== */

FUNC(StatusType, OS_CODE) ActivateTask(TaskType TaskID)
{
    if (TaskID >= (TaskType)OS_NUM_TASKS)
    {
        ErrorHook(E_OS_ID);
        return E_OS_ID;
    }
    /* On host: dispatch immediately (simplified, non-preemptive) */
    Os_TaskTable[TaskID]();
    return (StatusType)E_OK;
}

/* =========================================================================
 * GetTaskID
 * ====================================================================== */

FUNC(StatusType, OS_CODE) GetTaskID(TaskRefType TaskID)
{
    if (TaskID == NULL_PTR)
    {
        return E_OS_ID;
    }
    *TaskID = Os_CurrentTask;
    return (StatusType)E_OK;
}

/* =========================================================================
 * GetTaskState
 * ====================================================================== */

FUNC(StatusType, OS_CODE) GetTaskState(TaskType TaskID, TaskStateRefType State)
{
    if (State == NULL_PTR)
    {
        return E_OS_ID;
    }
    if (TaskID >= (TaskType)OS_NUM_TASKS)
    {
        return E_OS_ID;
    }
    *State = (TaskID == Os_CurrentTask) ? RUNNING : SUSPENDED;
    return (StatusType)E_OK;
}
