#ifndef STUB_TRANSPORT_H
#define STUB_TRANSPORT_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Returns pointer to last frame passed to SomeIpSd_Transmit(). */
const uint8* StubTransport_GetLastFrame(void);

/** Returns the length of the last transmitted frame. */
uint32       StubTransport_GetLastFrameLen(void);

/** Returns total call count since last Reset. */
uint32       StubTransport_GetCallCount(void);

/** Resets captured frame, length, call count, and return value. */
void         StubTransport_Reset(void);

/** Forces SomeIpSd_Transmit() to return the given value on next call(s). */
void         StubTransport_SetReturnValue(Std_ReturnType ReturnValue);

#ifdef __cplusplus
}
#endif

#endif /* STUB_TRANSPORT_H */
