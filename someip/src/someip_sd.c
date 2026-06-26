/**
 * SOME/IP Service Discovery implementation.
 * Implements offer / find / subscribe per AUTOSAR PRS_SOMEIPServiceDiscovery R22-11.
 *
 * Note: transport (UDP multicast) is abstracted behind SomeIpSd_Transmit()
 * which must be provided by the BSW integration layer.
 */

#include "someip_sd.h"
#include "someip_serializer.h"
#include <string.h>

/* -----------------------------------------------------------------------
 * Platform stub – replace with actual UDP/BSW send in integration
 * ----------------------------------------------------------------------- */
extern Std_ReturnType SomeIpSd_Transmit(const uint8_t *buf, uint32_t len);

/* -----------------------------------------------------------------------
 * Internal constants
 * ----------------------------------------------------------------------- */
#define SD_PAYLOAD_ENTRIES_LEN_OFFSET  4u   /* offset of entries array length */
#define SD_ENTRY_TYPE1_SIZE           16u
#define SD_ENTRY_TYPE2_SIZE           16u
#define SD_FLAGS_OFFSET                0u
#define SD_RESERVED_SIZE               3u

/* Maximum serialized SD payload: flags(1)+reserved(3)+entries_len(4)+
 * SOMEIP_SD_MAX_ENTRIES*16 + options_len(4)                          */
#define SD_MAX_PAYLOAD_SIZE  (8u + (SOMEIP_SD_MAX_ENTRIES * SD_ENTRY_TYPE1_SIZE) + 4u)
#define SD_FRAME_MAX_SIZE    (SOMEIP_HEADER_SIZE + SD_MAX_PAYLOAD_SIZE)

/* -----------------------------------------------------------------------
 * Internal helpers
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

/* -----------------------------------------------------------------------
 * Build and transmit an SD frame containing one Type-1 service entry
 * ----------------------------------------------------------------------- */
static Std_ReturnType sd_send_service_entry(
    uint8_t              entry_type,
    SomeIp_ServiceId_t   service_id,
    SomeIp_InstanceId_t  instance_id,
    uint8_t              major_ver,
    uint32_t             minor_ver,
    uint32_t             ttl_sec)
{
    static uint16_t session_id = 1u;  /* monotonically increasing per PRS_SOMEIPSD_00056 */

    uint8_t  frame[SD_FRAME_MAX_SIZE];
    uint8_t  payload[SD_MAX_PAYLOAD_SIZE];
    uint32_t payload_off = 0u;
    uint32_t out_len     = 0u;
    SomeIp_Message_t msg;

    (void)memset(frame,   0, sizeof(frame));
    (void)memset(payload, 0, sizeof(payload));

    /* SD payload: Flags (1) + Reserved (3) */
    payload[payload_off]  = SOMEIP_SD_FLAG_REBOOT;  /* set on first boot */
    payload_off          += 4u;                     /* flags + 3 reserved */

    /* Entries array length (= 1 entry * 16 bytes) */
    write_u32_be(&payload[payload_off], SD_ENTRY_TYPE1_SIZE);
    payload_off += 4u;

    /* Type-1 entry */
    payload[payload_off++] = entry_type;
    payload[payload_off++] = 0x00u;   /* index first options  */
    payload[payload_off++] = 0x00u;   /* index second options */
    payload[payload_off++] = 0x00u;   /* num opts 1 / num opts 2 */
    write_u16_be(&payload[payload_off], service_id);   payload_off += 2u;
    write_u16_be(&payload[payload_off], instance_id);  payload_off += 2u;
    payload[payload_off++] = major_ver;
    payload[payload_off++] = (uint8_t)((ttl_sec >> 16u) & 0xFFu);
    payload[payload_off++] = (uint8_t)((ttl_sec >>  8u) & 0xFFu);
    payload[payload_off++] = (uint8_t)( ttl_sec          & 0xFFu);
    write_u32_be(&payload[payload_off], minor_ver);    payload_off += 4u;

    /* Options array length = 0 */
    write_u32_be(&payload[payload_off], 0u);
    payload_off += 4u;

    /* Build SOME/IP header */
    (void)memset(&msg, 0, sizeof(msg));
    msg.header.service_id        = SOMEIP_SD_SERVICE_ID;
    msg.header.method_id         = SOMEIP_SD_METHOD_ID;
    msg.header.client_id         = 0x0000u;
    msg.header.session_id        = session_id++;
    msg.header.protocol_version  = SOMEIP_PROTOCOL_VERSION;
    msg.header.interface_version = SOMEIP_INTERFACE_VERSION;
    msg.header.message_type      = (uint8_t)SOMEIP_MSG_NOTIFICATION;
    msg.header.return_code       = (uint8_t)SOMEIP_RC_OK;
    msg.payload                  = payload;
    msg.payload_length           = payload_off;

    if (SomeIp_Serialize(&msg, frame, sizeof(frame), &out_len) != E_OK) {
        return E_NOT_OK;
    }

    return SomeIpSd_Transmit(frame, out_len);
}

/* -----------------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------------- */

Std_ReturnType SomeIpSd_OfferService(
    SomeIp_ServiceId_t   service_id,
    SomeIp_InstanceId_t  instance_id,
    uint8_t              major_ver,
    uint32_t             minor_ver,
    uint32_t             ttl_sec)
{
    return sd_send_service_entry(
        (uint8_t)SOMEIP_SD_ENTRY_OFFER_SERVICE,
        service_id, instance_id, major_ver, minor_ver, ttl_sec);
}

Std_ReturnType SomeIpSd_FindService(
    SomeIp_ServiceId_t   service_id,
    SomeIp_InstanceId_t  instance_id,
    uint8_t              major_ver,
    uint32_t             minor_ver)
{
    return sd_send_service_entry(
        (uint8_t)SOMEIP_SD_ENTRY_FIND_SERVICE,
        service_id, instance_id, major_ver, minor_ver, 0xFFFFFFu);
}

Std_ReturnType SomeIpSd_SubscribeEventgroup(
    SomeIp_ServiceId_t      service_id,
    SomeIp_InstanceId_t     instance_id,
    SomeIp_EventGroupId_t   eventgroup_id,
    uint8_t                 major_ver,
    uint32_t                ttl_sec)
{
    static uint16_t session_id = 1u;

    uint8_t  frame[SD_FRAME_MAX_SIZE];
    uint8_t  payload[SD_MAX_PAYLOAD_SIZE];
    uint32_t payload_off = 0u;
    uint32_t out_len     = 0u;
    SomeIp_Message_t msg;

    (void)memset(frame,   0, sizeof(frame));
    (void)memset(payload, 0, sizeof(payload));

    /* SD payload header */
    payload[payload_off]  = SOMEIP_SD_FLAG_REBOOT;
    payload_off          += 4u;

    /* Entries array length (1 Type-2 entry = 16 bytes) */
    write_u32_be(&payload[payload_off], SD_ENTRY_TYPE2_SIZE);
    payload_off += 4u;

    /* Type-2 entry */
    payload[payload_off++] = (uint8_t)SOMEIP_SD_ENTRY_SUBSCRIBE_EVENTGROUP;
    payload[payload_off++] = 0x00u;
    payload[payload_off++] = 0x00u;
    payload[payload_off++] = 0x00u;
    write_u16_be(&payload[payload_off], service_id);    payload_off += 2u;
    write_u16_be(&payload[payload_off], instance_id);   payload_off += 2u;
    payload[payload_off++] = major_ver;
    payload[payload_off++] = (uint8_t)((ttl_sec >> 16u) & 0xFFu);
    payload[payload_off++] = (uint8_t)((ttl_sec >>  8u) & 0xFFu);
    payload[payload_off++] = (uint8_t)( ttl_sec          & 0xFFu);
    payload[payload_off++] = 0x00u;  /* reserved */
    payload[payload_off++] = 0x00u;  /* counter  */
    write_u16_be(&payload[payload_off], eventgroup_id); payload_off += 2u;

    /* Options array length = 0 */
    write_u32_be(&payload[payload_off], 0u);
    payload_off += 4u;

    (void)memset(&msg, 0, sizeof(msg));
    msg.header.service_id        = SOMEIP_SD_SERVICE_ID;
    msg.header.method_id         = SOMEIP_SD_METHOD_ID;
    msg.header.client_id         = 0x0000u;
    msg.header.session_id        = session_id++;
    msg.header.protocol_version  = SOMEIP_PROTOCOL_VERSION;
    msg.header.interface_version = SOMEIP_INTERFACE_VERSION;
    msg.header.message_type      = (uint8_t)SOMEIP_MSG_NOTIFICATION;
    msg.header.return_code       = (uint8_t)SOMEIP_RC_OK;
    msg.payload                  = payload;
    msg.payload_length           = payload_off;

    if (SomeIp_Serialize(&msg, frame, sizeof(frame), &out_len) != E_OK) {
        return E_NOT_OK;
    }

    return SomeIpSd_Transmit(frame, out_len);
}

Std_ReturnType SomeIpSd_RxIndication(const uint8_t *buf, uint32_t length)
{
    SomeIp_Message_t msg;
    uint32_t         entries_len;
    uint32_t         offset;
    uint8_t          entry_type;

    if (SomeIp_Deserialize(buf, length, &msg) != E_OK) {
        return E_NOT_OK;
    }

    if ((msg.header.service_id != SOMEIP_SD_SERVICE_ID) ||
        (msg.header.method_id  != SOMEIP_SD_METHOD_ID)) {
        return E_NOT_OK;
    }

    if (msg.payload == NULL || msg.payload_length < 8u) {
        return E_NOT_OK;
    }

    /* Skip flags (1) + reserved (3) */
    offset = 4u;

    /* Read entries array length */
    entries_len  = ((uint32_t)msg.payload[offset]     << 24u);
    entries_len |= ((uint32_t)msg.payload[offset + 1] << 16u);
    entries_len |= ((uint32_t)msg.payload[offset + 2] <<  8u);
    entries_len |=  (uint32_t)msg.payload[offset + 3];
    offset += 4u;

    /* Iterate entries – minimal parsing for demonstration */
    while (entries_len >= SD_ENTRY_TYPE1_SIZE) {
        if (offset + SD_ENTRY_TYPE1_SIZE > msg.payload_length) {
            break;
        }

        entry_type = msg.payload[offset];

        /*
         * Extended processing (e.g. registering discovered services,
         * acknowledging subscriptions) would be dispatched here based
         * on entry_type.
         */
        (void)entry_type;

        offset      += SD_ENTRY_TYPE1_SIZE;
        entries_len -= SD_ENTRY_TYPE1_SIZE;
    }

    return E_OK;
}
