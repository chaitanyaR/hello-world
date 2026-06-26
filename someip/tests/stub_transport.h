#ifndef STUB_TRANSPORT_H
#define STUB_TRANSPORT_H

#include "someip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

const uint8_t *StubTransport_GetLastFrame(void);
uint32_t       StubTransport_GetLastFrameLen(void);
uint32_t       StubTransport_GetCallCount(void);
void           StubTransport_Reset(void);
void           StubTransport_SetReturnValue(Std_ReturnType ret);

#ifdef __cplusplus
}
#endif

#endif /* STUB_TRANSPORT_H */
