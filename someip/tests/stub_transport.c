/**
 * stub_transport.c – Test stub for SomeIpSd_Transmit()
 *
 * Captures every frame passed to the platform transport function so GTest
 * assertions can inspect wire-level encoding without a real UDP socket.
 */

#include "stub_transport.h"
#include <string.h>

#define STUB_BUF_SIZE   1024u

static uint8          StubFrame[STUB_BUF_SIZE];
static uint32         StubFrameLen  = 0u;
static uint32         StubCallCount = 0u;
static Std_ReturnType StubRetVal    = E_OK;

/* Called by SomeIp_SD.c */
Std_ReturnType SomeIpSd_Transmit(const uint8* BufPtr, uint32 Length)
{
    StubCallCount++;
    if ((BufPtr != NULL) && (Length <= STUB_BUF_SIZE))
    {
        (void)memcpy(StubFrame, BufPtr, (size_t)Length);
        StubFrameLen = Length;
    }
    return StubRetVal;
}

const uint8* StubTransport_GetLastFrame(void)    { return StubFrame; }
uint32       StubTransport_GetLastFrameLen(void) { return StubFrameLen; }
uint32       StubTransport_GetCallCount(void)    { return StubCallCount; }

void StubTransport_Reset(void)
{
    (void)memset(StubFrame, 0, STUB_BUF_SIZE);
    StubFrameLen  = 0u;
    StubCallCount = 0u;
    StubRetVal    = E_OK;
}

void StubTransport_SetReturnValue(Std_ReturnType ReturnValue)
{
    StubRetVal = ReturnValue;
}
