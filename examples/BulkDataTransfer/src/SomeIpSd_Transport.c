/**
 * SomeIpSd_Transport.c – SOME/IP SD Transport Stub
 * AUTOSAR Release R22-11
 *
 * Provides the SomeIpSd_Transmit() BSW service expected by SomeIp_SD.c.
 * In a full AUTOSAR stack this would be implemented by SoAd (Socket Adaptor)
 * and would send the SD frame over UDP multicast.
 *
 * For the host simulation it logs the outgoing frame to stdout.
 */

#include "Std_Types.h"
#include "Compiler.h"

#include <stdio.h>

/* Forward declaration: SomeIp_SD.c declares this via extern; no AUTOSAR header
 * exists for BSW integration points — prototype provided here to satisfy
 * -Wmissing-prototypes. */
Std_ReturnType SomeIpSd_Transmit(const uint8 *BufPtr, uint32 Length);

/**
 * SomeIpSd_Transmit
 * Called by SomeIp_SD.c to transmit a serialised SD frame.
 *
 * @param BufPtr  Pointer to the frame data.
 * @param Length  Length of the frame in bytes.
 * @return E_OK always (simulated successful transmission).
 */
Std_ReturnType SomeIpSd_Transmit(const uint8 *BufPtr, uint32 Length)
{
    uint32 i;

    printf("[SoAd] SomeIpSd_Transmit: %u bytes\n", (unsigned)Length);
    printf("[SoAd]  ");
    for (i = 0u; i < Length && i < 32u; i++)
    {
        printf("%02X ", (unsigned)BufPtr[i]);
    }
    if (Length > 32u)
    {
        printf("...");
    }
    printf("\n");

    return E_OK;
}
