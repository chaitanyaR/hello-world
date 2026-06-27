/**
 * main.c – ECU Main / BSW Initialisation Sequence
 * AUTOSAR Release R22-11
 *
 * Host-simulation entry point for the BulkDataTransfer example application.
 *
 * Startup sequence (mirrors AUTOSAR ECU startup):
 *   1. Hardware initialisation (stubbed on host)
 *   2. BSW module initialisation: SomeIp_Init, SomeIpSd_Init
 *   3. RTE start: Rte_Start()
 *   4. SWC init runnable: BulkDataTransfer_InitRunnable()
 *   5. Inject simulation data into the RTE R-Port receive buffer
 *   6. StartOS(OSDEFAULTAPPMODE) – runs the OS scheduler (simulated as a
 *      fixed number of 10 ms cycles for the host simulation build)
 *
 * OS Hooks implemented here:
 *   StartupHook, ShutdownHook, ErrorHook, PreTaskHook, PostTaskHook
 */

#include "Std_Types.h"
#include "Compiler.h"
#include "SomeIp.h"
#include "SomeIp_SD.h"
#include "Rte_BulkDataTransfer.h"
#include "BulkDataTransfer.h"
#include "Os.h"
#include "Os_Cfg.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* =========================================================================
 * Simulation parameters
 * ====================================================================== */

/** Number of 10 ms cycles the simulation runs before shutdown. */
#define SIM_CYCLE_COUNT   10u

/* =========================================================================
 * Simulation: CRC-32 helper (mirrors the one in BulkDataTransfer.c)
 * This is used only to seed the R-Port receive buffer with valid data.
 * ====================================================================== */

#define CRC32_INIT_VALUE   0xFFFFFFFFUL
#define CRC32_FINAL_XOR    0xFFFFFFFFUL
#define CRC32_POLYNOMIAL   0xEDB88320UL

static uint32 Sim_Crc32(const uint8 *data, uint32 length)
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
    return crc ^ CRC32_FINAL_XOR;
}

/* =========================================================================
 * OS Hook implementations
 * ====================================================================== */

FUNC(void, OS_CODE) StartupHook(void)
{
    printf("[OS] StartupHook: OS started in OSDEFAULTAPPMODE\n");
}

FUNC(void, OS_CODE) ShutdownHook(StatusType Error)
{
    printf("[OS] ShutdownHook: OS shutting down, error=0x%02X\n",
           (unsigned)Error);
}

FUNC(void, OS_CODE) ErrorHook(StatusType Error)
{
    printf("[OS] ErrorHook: OS service error=0x%02X\n", (unsigned)Error);
}

FUNC(void, OS_CODE) PreTaskHook(void)
{
    /* Nothing to do in simulation */
}

FUNC(void, OS_CODE) PostTaskHook(void)
{
    /* Nothing to do in simulation */
}

/* =========================================================================
 * main – ECU entry point
 * ====================================================================== */

int main(void)
{
    BulkDataBufferType   simBuffer;
    BulkDataChecksumType simChecksum;
    uint32               i;

    printf("=============================================================\n");
    printf(" BulkDataTransfer – AUTOSAR R22-11 Example Application\n");
    printf("=============================================================\n\n");

    /* ------------------------------------------------------------------
     * Step 1: BSW initialisation
     * ------------------------------------------------------------------ */
    printf("[BSW] Initialising SOME/IP Transformer...\n");
    SomeIp_Init(NULL_PTR);

    printf("[BSW] Initialising SOME/IP Service Discovery...\n");
    SomeIpSd_Init(NULL_PTR);

    /* ------------------------------------------------------------------
     * Step 2: RTE start
     * ------------------------------------------------------------------ */
    printf("[RTE] Starting RTE...\n");
    if (Rte_Start() != E_OK)
    {
        printf("[RTE] ERROR: Rte_Start failed – aborting.\n");
        return 1;
    }

    /* ------------------------------------------------------------------
     * Step 3: SWC init runnable (INIT-EVENT)
     * ------------------------------------------------------------------ */
    printf("[SWC] Executing BulkDataTransfer_InitRunnable...\n\n");
    BulkDataTransfer_InitRunnable();

    /* ------------------------------------------------------------------
     * Step 4: Seed simulation input data into the RTE R-Port buffer.
     * Fill buffer with incrementing byte pattern 0x00..0xFF repeated,
     * then compute a valid CRC-32 so the runnable accepts it.
     * ------------------------------------------------------------------ */
    printf("[SIM] Injecting test data into R-Port receive buffer...\n");
    for (i = 0u; i < BULK_DATA_BUFFER_SIZE; i++)
    {
        simBuffer[i] = (uint8)(i & 0xFFu);
    }
    simChecksum = (BulkDataChecksumType)Sim_Crc32(simBuffer, BULK_DATA_BUFFER_SIZE);
    printf("[SIM] Injected buffer pattern 0x00..0xFF, CRC=0x%08X\n\n",
           (unsigned)simChecksum);

    Rte_Sim_InjectBulkBuffer(simBuffer, simChecksum);

    /* ------------------------------------------------------------------
     * Step 5: StartOS – triggers OsAlarm_10ms which activates OsTask_10ms
     * The OS stub in Os.c runs SIM_CYCLE_COUNT 10 ms cycles then returns.
     * ------------------------------------------------------------------ */
    printf("[OS]  Starting OS (OSDEFAULTAPPMODE), %u × 10 ms cycles...\n\n",
           (unsigned)SIM_CYCLE_COUNT);
    StartOS(OSDEFAULTAPPMODE);

    /* ------------------------------------------------------------------
     * Step 6: Verify output data written to P-Port
     * ------------------------------------------------------------------ */
    {
        BulkDataBufferType   txBuffer;
        BulkDataChecksumType txChecksum;
        BulkDataChecksumType verifyChecksum;
        boolean              pass = TRUE;

        Rte_Sim_ReadTxBulkBuffer(txBuffer, &txChecksum);
        verifyChecksum = (BulkDataChecksumType)Sim_Crc32(txBuffer, BULK_DATA_BUFFER_SIZE);

        printf("\n[SIM] Verifying P-Port transmit data...\n");
        printf("[SIM] P-Port CRC read from port : 0x%08X\n", (unsigned)txChecksum);
        printf("[SIM] P-Port CRC computed       : 0x%08X\n", (unsigned)verifyChecksum);

        if (txChecksum != verifyChecksum)
        {
            printf("[SIM] FAIL: CRC mismatch on output buffer!\n");
            pass = FALSE;
        }

        /* Verify transformation: every byte should be the bitwise inverse */
        for (i = 0u; i < BULK_DATA_BUFFER_SIZE; i++)
        {
            if (txBuffer[i] != (uint8)(~simBuffer[i]))
            {
                printf("[SIM] FAIL: byte[%u] expected 0x%02X got 0x%02X\n",
                       (unsigned)i,
                       (unsigned)(uint8)(~simBuffer[i]),
                       (unsigned)txBuffer[i]);
                pass = FALSE;
                break;
            }
        }

        if (pass == TRUE)
        {
            printf("[SIM] PASS: output buffer verified (byte-inversion + CRC OK)\n");
        }
    }

    /* ------------------------------------------------------------------
     * Step 7: Shutdown
     * ------------------------------------------------------------------ */
    printf("\n[BSW] Shutting down...\n");
    Rte_Stop();
    ShutdownOS((StatusType)E_OK);

    printf("\n=============================================================\n");
    printf(" Simulation complete.\n");
    printf("=============================================================\n");
    return 0;
}
