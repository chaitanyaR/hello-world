/**
 * SOME/IP serializer / deserializer interface.
 * Handles big-endian wire encoding per AUTOSAR PRS_SOMEIPProtocol R22-11.
 */

#ifndef SOMEIP_SERIALIZER_H
#define SOMEIP_SERIALIZER_H

#include "someip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Serialize a SomeIp_Message_t into a flat byte buffer.
 *
 * @param msg        Message to serialize.
 * @param buf        Output buffer.
 * @param buf_size   Size of output buffer in bytes.
 * @param out_len    Bytes written on success.
 * @return E_OK / E_NOT_OK.
 */
Std_ReturnType SomeIp_Serialize(
    const SomeIp_Message_t *msg,
    uint8_t                *buf,
    uint32_t                buf_size,
    uint32_t               *out_len);

/**
 * Deserialize a raw byte buffer into a SomeIp_Message_t.
 * The payload pointer in the output message points into buf (zero-copy).
 *
 * @param buf        Raw SOME/IP frame.
 * @param buf_len    Frame length in bytes.
 * @param msg        Output message structure.
 * @return E_OK / E_NOT_OK.
 */
Std_ReturnType SomeIp_Deserialize(
    const uint8_t   *buf,
    uint32_t         buf_len,
    SomeIp_Message_t *msg);

/**
 * Validate header fields against AUTOSAR constraints.
 *
 * @param header Header to validate.
 * @return E_OK if valid, E_NOT_OK otherwise.
 */
Std_ReturnType SomeIp_ValidateHeader(const SomeIp_Header_t *header);

#ifdef __cplusplus
}
#endif

#endif /* SOMEIP_SERIALIZER_H */
