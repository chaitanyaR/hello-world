/**
 * Test stub for SomeIpSd_Transmit().
 * Captures the last transmitted frame so GTest assertions can inspect it.
 */

#include "someip_types.h"
#include <string.h>
#include <stdint.h>

#define STUB_BUF_SIZE 1024u

static uint8_t  g_last_frame[STUB_BUF_SIZE];
static uint32_t g_last_frame_len  = 0u;
static uint32_t g_transmit_calls  = 0u;
static Std_ReturnType g_return_val = 0u; /* E_OK */

/* Called by someip_sd.c */
Std_ReturnType SomeIpSd_Transmit(const uint8_t *buf, uint32_t len)
{
    g_transmit_calls++;
    if (len <= STUB_BUF_SIZE && buf != NULL) {
        memcpy(g_last_frame, buf, len);
        g_last_frame_len = len;
    }
    return g_return_val;
}

/* Test helper access functions */
const uint8_t *StubTransport_GetLastFrame(void)  { return g_last_frame; }
uint32_t       StubTransport_GetLastFrameLen(void){ return g_last_frame_len; }
uint32_t       StubTransport_GetCallCount(void)   { return g_transmit_calls; }

void StubTransport_Reset(void)
{
    memset(g_last_frame, 0, STUB_BUF_SIZE);
    g_last_frame_len = 0u;
    g_transmit_calls = 0u;
    g_return_val     = 0u; /* E_OK */
}

void StubTransport_SetReturnValue(Std_ReturnType ret) { g_return_val = ret; }
