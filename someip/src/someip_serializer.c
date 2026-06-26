/**
 * SOME/IP serializer / deserializer implementation.
 * Wire format is big-endian per AUTOSAR PRS_SOMEIPProtocol R22-11.
 */

#include "someip_serializer.h"
#include <string.h>

/* -----------------------------------------------------------------------
 * Internal helpers – big-endian read/write
 * ----------------------------------------------------------------------- */

static void write_u16_be(uint8_t *buf, uint16_t val)
{
    buf[0] = (uint8_t)(val >> 8u);
    buf[1] = (uint8_t)(val & 0xFFu);
}

static void write_u32_be(uint8_t *buf, uint32_t val)
{
    buf[0] = (uint8_t)(val >> 24u);
    buf[1] = (uint8_t)(val >> 16u);
    buf[2] = (uint8_t)(val >>  8u);
    buf[3] = (uint8_t)(val & 0xFFu);
}

static uint16_t read_u16_be(const uint8_t *buf)
{
    return (uint16_t)(((uint16_t)buf[0] << 8u) | (uint16_t)buf[1]);
}

static uint32_t read_u32_be(const uint8_t *buf)
{
    return ((uint32_t)buf[0] << 24u) |
           ((uint32_t)buf[1] << 16u) |
           ((uint32_t)buf[2] <<  8u) |
            (uint32_t)buf[3];
}

/* -----------------------------------------------------------------------
 * SomeIp_ValidateHeader
 * ----------------------------------------------------------------------- */

Std_ReturnType SomeIp_ValidateHeader(const SomeIp_Header_t *header)
{
    if (header == NULL) {
        return E_NOT_OK;
    }

    /* Protocol version must be 0x01 per PRS_SOMEIP_00052 */
    if (header->protocol_version != SOMEIP_PROTOCOL_VERSION) {
        return E_NOT_OK;
    }

    /* Length field must cover at least the remaining 8 header bytes */
    if (header->length < 8u) {
        return E_NOT_OK;
    }

    /* Validate message type range */
    switch ((SomeIp_MessageType_t)header->message_type) {
        case SOMEIP_MSG_REQUEST:
        case SOMEIP_MSG_REQUEST_NO_RETURN:
        case SOMEIP_MSG_NOTIFICATION:
        case SOMEIP_MSG_RESPONSE:
        case SOMEIP_MSG_ERROR:
        case SOMEIP_MSG_TP_REQUEST:
        case SOMEIP_MSG_TP_REQUEST_NO_RETURN:
        case SOMEIP_MSG_TP_NOTIFICATION:
        case SOMEIP_MSG_TP_RESPONSE:
        case SOMEIP_MSG_TP_ERROR:
            break;
        default:
            return E_NOT_OK;
    }

    return E_OK;
}

/* -----------------------------------------------------------------------
 * SomeIp_Serialize
 * ----------------------------------------------------------------------- */

Std_ReturnType SomeIp_Serialize(
    const SomeIp_Message_t *msg,
    uint8_t                *buf,
    uint32_t                buf_size,
    uint32_t               *out_len)
{
    uint32_t total;

    if ((msg == NULL) || (buf == NULL) || (out_len == NULL)) {
        return E_NOT_OK;
    }

    /* Total frame = 16-byte header + payload */
    total = SOMEIP_HEADER_SIZE + msg->payload_length;

    if (buf_size < total) {
        return E_NOT_OK;
    }

    /* Serialize header fields in big-endian order */
    write_u16_be(&buf[0],  msg->header.service_id);
    write_u16_be(&buf[2],  msg->header.method_id);
    /* Length = remaining header bytes (8) + payload */
    write_u32_be(&buf[4],  8u + msg->payload_length);
    write_u16_be(&buf[8],  msg->header.client_id);
    write_u16_be(&buf[10], msg->header.session_id);
    buf[12] = msg->header.protocol_version;
    buf[13] = msg->header.interface_version;
    buf[14] = msg->header.message_type;
    buf[15] = msg->header.return_code;

    /* Copy payload */
    if ((msg->payload_length > 0u) && (msg->payload != NULL)) {
        (void)memcpy(&buf[SOMEIP_HEADER_SIZE], msg->payload, msg->payload_length);
    }

    *out_len = total;
    return E_OK;
}

/* -----------------------------------------------------------------------
 * SomeIp_Deserialize
 * ----------------------------------------------------------------------- */

Std_ReturnType SomeIp_Deserialize(
    const uint8_t    *buf,
    uint32_t          buf_len,
    SomeIp_Message_t *msg)
{
    uint32_t length_field;
    uint32_t payload_len;

    if ((buf == NULL) || (msg == NULL)) {
        return E_NOT_OK;
    }

    if (buf_len < SOMEIP_HEADER_SIZE) {
        return E_NOT_OK;
    }

    /* Parse header */
    msg->header.service_id        = read_u16_be(&buf[0]);
    msg->header.method_id         = read_u16_be(&buf[2]);
    length_field                  = read_u32_be(&buf[4]);
    msg->header.length            = length_field;
    msg->header.client_id         = read_u16_be(&buf[8]);
    msg->header.session_id        = read_u16_be(&buf[10]);
    msg->header.protocol_version  = buf[12];
    msg->header.interface_version = buf[13];
    msg->header.message_type      = buf[14];
    msg->header.return_code       = buf[15];

    /* length_field = 8 (remaining header) + payload; must be >= 8 */
    if (length_field < 8u) {
        return E_NOT_OK;
    }

    payload_len = length_field - 8u;

    /* Ensure buffer contains the advertised payload */
    if (buf_len < SOMEIP_HEADER_SIZE + payload_len) {
        return E_NOT_OK;
    }

    msg->payload        = (payload_len > 0u) ? &buf[SOMEIP_HEADER_SIZE] : NULL;
    msg->payload_length = payload_len;

    return SomeIp_ValidateHeader(&msg->header);
}
