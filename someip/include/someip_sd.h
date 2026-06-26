/**
 * SOME/IP Service Discovery (SD) interface.
 * Implements offer / find / subscribe / subscribe-ack per
 * AUTOSAR PRS_SOMEIPServiceDiscovery R22-11.
 */

#ifndef SOMEIP_SD_H
#define SOMEIP_SD_H

#include "someip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SD Service ID and Method ID are fixed per spec */
#define SOMEIP_SD_SERVICE_ID  0xFFFFu
#define SOMEIP_SD_METHOD_ID   0x8100u

/* -----------------------------------------------------------------------
 * SD Entry types (AUTOSAR PRS_SOMEIPSD_00009)
 * ----------------------------------------------------------------------- */
typedef enum {
    SOMEIP_SD_ENTRY_FIND_SERVICE        = 0x00,
    SOMEIP_SD_ENTRY_OFFER_SERVICE       = 0x01,
    SOMEIP_SD_ENTRY_SUBSCRIBE_EVENTGROUP  = 0x06,
    SOMEIP_SD_ENTRY_SUBSCRIBE_EVENTGROUP_ACK = 0x07
} SomeIpSd_EntryType_t;

/* -----------------------------------------------------------------------
 * SD flags (AUTOSAR PRS_SOMEIPSD_00049)
 * ----------------------------------------------------------------------- */
#define SOMEIP_SD_FLAG_REBOOT   0x80u
#define SOMEIP_SD_FLAG_UNICAST  0x40u

/* -----------------------------------------------------------------------
 * SD Type-1 entry (Find / Offer service)
 * ----------------------------------------------------------------------- */
typedef struct {
    uint8_t                 type;           /* SomeIpSd_EntryType_t     */
    uint8_t                 index_first;    /* First options index       */
    uint8_t                 index_second;   /* Second options index      */
    uint8_t                 num_opts;       /* Option counts (packed)    */
    SomeIp_ServiceId_t      service_id;
    SomeIp_InstanceId_t     instance_id;
    uint8_t                 major_version;
    uint8_t                 ttl[3];         /* 24-bit TTL in seconds     */
    uint32_t                minor_version;
} SomeIpSd_ServiceEntry_t;

/* -----------------------------------------------------------------------
 * SD Type-2 entry (Subscribe / Subscribe-Ack eventgroup)
 * ----------------------------------------------------------------------- */
typedef struct {
    uint8_t                 type;           /* SomeIpSd_EntryType_t     */
    uint8_t                 index_first;
    uint8_t                 index_second;
    uint8_t                 num_opts;
    SomeIp_ServiceId_t      service_id;
    SomeIp_InstanceId_t     instance_id;
    uint8_t                 major_version;
    uint8_t                 ttl[3];
    uint8_t                 reserved;
    uint8_t                 counter;        /* Counter field (4-bit)     */
    SomeIp_EventGroupId_t   eventgroup_id;
} SomeIpSd_EventgroupEntry_t;

/* -----------------------------------------------------------------------
 * SD message container
 * ----------------------------------------------------------------------- */
#define SOMEIP_SD_MAX_ENTRIES  16u
#define SOMEIP_SD_MAX_OPTIONS  16u

typedef struct {
    uint8_t                    flags;
    uint8_t                    reserved[3];
    SomeIpSd_ServiceEntry_t    entries[SOMEIP_SD_MAX_ENTRIES];
    uint8_t                    num_entries;
} SomeIpSd_Message_t;

/* -----------------------------------------------------------------------
 * SD API
 * ----------------------------------------------------------------------- */

/**
 * Transmit an Offer Service SD message for the given service/instance.
 *
 * @param service_id   Service identifier.
 * @param instance_id  Instance identifier.
 * @param major_ver    Major version number.
 * @param minor_ver    Minor version number.
 * @param ttl_sec      Time-to-live in seconds (0 = StopOffer).
 * @return E_OK / E_NOT_OK.
 */
Std_ReturnType SomeIpSd_OfferService(
    SomeIp_ServiceId_t   service_id,
    SomeIp_InstanceId_t  instance_id,
    uint8_t              major_ver,
    uint32_t             minor_ver,
    uint32_t             ttl_sec);

/**
 * Transmit a Find Service SD message.
 *
 * @param service_id   Service identifier.
 * @param instance_id  Instance identifier (0xFFFF = any).
 * @param major_ver    Major version (0xFF = any).
 * @param minor_ver    Minor version (0xFFFFFFFF = any).
 * @return E_OK / E_NOT_OK.
 */
Std_ReturnType SomeIpSd_FindService(
    SomeIp_ServiceId_t   service_id,
    SomeIp_InstanceId_t  instance_id,
    uint8_t              major_ver,
    uint32_t             minor_ver);

/**
 * Subscribe to an eventgroup.
 *
 * @param service_id     Service identifier.
 * @param instance_id    Instance identifier.
 * @param eventgroup_id  Eventgroup identifier.
 * @param major_ver      Major version.
 * @param ttl_sec        Subscription TTL (0 = unsubscribe).
 * @return E_OK / E_NOT_OK.
 */
Std_ReturnType SomeIpSd_SubscribeEventgroup(
    SomeIp_ServiceId_t      service_id,
    SomeIp_InstanceId_t     instance_id,
    SomeIp_EventGroupId_t   eventgroup_id,
    uint8_t                 major_ver,
    uint32_t                ttl_sec);

/**
 * Process an incoming raw SD frame.
 *
 * @param buf     Raw frame bytes.
 * @param length  Frame length.
 * @return E_OK / E_NOT_OK.
 */
Std_ReturnType SomeIpSd_RxIndication(const uint8_t *buf, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* SOMEIP_SD_H */
