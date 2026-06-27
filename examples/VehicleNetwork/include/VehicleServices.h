/**
 * VehicleServices.h – Vehicle Network Service and Node Definitions
 * AUTOSAR Release R22-11
 *
 * Shared constants for all five ECU nodes in the vehicle Ethernet network.
 * Defines SOME/IP Service IDs, Instance IDs, EventGroup IDs, and the UDP
 * port assignments used by the host-simulation transport layer.
 *
 * Network:  5-node vehicle Ethernet, 100BASE-T1 switch
 * Protocol: SOME/IP + SOME/IP-SD (PRS_SOMEIPServiceDiscovery R22-11)
 */

#ifndef VEHICLE_SERVICES_H
#define VEHICLE_SERVICES_H

#include "SomeIp_Types.h"

/* =========================================================================
 * Node identifiers (ECU IDs mapped as SOME/IP ClientID base)
 * ====================================================================== */
#define NODE_ID_BCM      ((uint8)0x01u)   /* Body Control Module    */
#define NODE_ID_ECM      ((uint8)0x02u)   /* Engine Control Module  */
#define NODE_ID_ADAS     ((uint8)0x03u)   /* ADAS Controller        */
#define NODE_ID_GATEWAY  ((uint8)0x04u)   /* Domain Gateway         */
#define NODE_ID_IPC      ((uint8)0x05u)   /* Instrument Cluster     */

/* =========================================================================
 * UDP port assignments (host simulation – loopback 127.0.0.1)
 * In production these map to SOME/IP-SD multicast 239.192.255.251:30490
 * ====================================================================== */
#define NODE_ADDR_LOOPBACK   "127.0.0.1"

#define NODE_PORT_BCM        ((uint16)30501u)
#define NODE_PORT_ECM        ((uint16)30502u)
#define NODE_PORT_ADAS       ((uint16)30503u)
#define NODE_PORT_GATEWAY    ((uint16)30504u)
#define NODE_PORT_IPC        ((uint16)30505u)

/* =========================================================================
 * BCM – Body Control Module services
 * Switch port 1 | 100BASE-T1 | 169.254.1.1 (production)
 * ====================================================================== */
/** DoorLock – controls vehicle door lock/unlock */
#define SOMEIP_SVC_DOOR_LOCK            ((SomeIp_ServiceIdType)0x0101u)
#define SOMEIP_SVC_DOOR_LOCK_INST       ((SomeIp_InstanceIdType)0x0001u)
#define SOMEIP_SVC_DOOR_LOCK_EG         ((SomeIp_EventGroupIdType)0x0001u)
#define SOMEIP_SVC_DOOR_LOCK_MAJOR      ((uint8)1u)
#define SOMEIP_SVC_DOOR_LOCK_MINOR      ((uint32)0u)

/** WindowControl – window raise/lower control */
#define SOMEIP_SVC_WINDOW_CTRL          ((SomeIp_ServiceIdType)0x0102u)
#define SOMEIP_SVC_WINDOW_CTRL_INST     ((SomeIp_InstanceIdType)0x0001u)
#define SOMEIP_SVC_WINDOW_CTRL_EG       ((SomeIp_EventGroupIdType)0x0001u)
#define SOMEIP_SVC_WINDOW_CTRL_MAJOR    ((uint8)1u)
#define SOMEIP_SVC_WINDOW_CTRL_MINOR    ((uint32)0u)

/** LightControl – exterior lighting control */
#define SOMEIP_SVC_LIGHT_CTRL           ((SomeIp_ServiceIdType)0x0103u)
#define SOMEIP_SVC_LIGHT_CTRL_INST      ((SomeIp_InstanceIdType)0x0001u)
#define SOMEIP_SVC_LIGHT_CTRL_EG        ((SomeIp_EventGroupIdType)0x0001u)
#define SOMEIP_SVC_LIGHT_CTRL_MAJOR     ((uint8)1u)
#define SOMEIP_SVC_LIGHT_CTRL_MINOR     ((uint32)0u)

/* =========================================================================
 * ECM – Engine Control Module services
 * Switch port 2 | 100BASE-T1 | 169.254.1.2 (production)
 * ====================================================================== */
/** EngineStatus – engine RPM, temperature, state notifications */
#define SOMEIP_SVC_ENGINE_STATUS        ((SomeIp_ServiceIdType)0x0201u)
#define SOMEIP_SVC_ENGINE_STATUS_INST   ((SomeIp_InstanceIdType)0x0001u)
#define SOMEIP_SVC_ENGINE_STATUS_EG     ((SomeIp_EventGroupIdType)0x0001u)
#define SOMEIP_SVC_ENGINE_STATUS_MAJOR  ((uint8)1u)
#define SOMEIP_SVC_ENGINE_STATUS_MINOR  ((uint32)0u)

/** ThrottleControl – throttle position command */
#define SOMEIP_SVC_THROTTLE_CTRL        ((SomeIp_ServiceIdType)0x0202u)
#define SOMEIP_SVC_THROTTLE_CTRL_INST   ((SomeIp_InstanceIdType)0x0001u)
#define SOMEIP_SVC_THROTTLE_CTRL_EG     ((SomeIp_EventGroupIdType)0x0001u)
#define SOMEIP_SVC_THROTTLE_CTRL_MAJOR  ((uint8)1u)
#define SOMEIP_SVC_THROTTLE_CTRL_MINOR  ((uint32)0u)

/* =========================================================================
 * ADAS – Advanced Driver Assistance System services
 * Switch port 3 | 100BASE-T1 | 169.254.1.3 (production)
 * ====================================================================== */
/** LaneKeeping – lane departure warning / assist */
#define SOMEIP_SVC_LANE_KEEPING         ((SomeIp_ServiceIdType)0x0301u)
#define SOMEIP_SVC_LANE_KEEPING_INST    ((SomeIp_InstanceIdType)0x0001u)
#define SOMEIP_SVC_LANE_KEEPING_EG      ((SomeIp_EventGroupIdType)0x0001u)
#define SOMEIP_SVC_LANE_KEEPING_MAJOR   ((uint8)1u)
#define SOMEIP_SVC_LANE_KEEPING_MINOR   ((uint32)0u)

/** CollisionWarning – forward collision warning */
#define SOMEIP_SVC_COLLISION_WARN       ((SomeIp_ServiceIdType)0x0302u)
#define SOMEIP_SVC_COLLISION_WARN_INST  ((SomeIp_InstanceIdType)0x0001u)
#define SOMEIP_SVC_COLLISION_WARN_EG    ((SomeIp_EventGroupIdType)0x0001u)
#define SOMEIP_SVC_COLLISION_WARN_MAJOR ((uint8)1u)
#define SOMEIP_SVC_COLLISION_WARN_MINOR ((uint32)0u)

/* =========================================================================
 * Gateway – Domain Gateway services
 * Switch port 4 | 100BASE-T1 | 169.254.1.4 (production)
 * ====================================================================== */
/** GatewayRouting – cross-domain service routing/proxying */
#define SOMEIP_SVC_GW_ROUTING           ((SomeIp_ServiceIdType)0x0401u)
#define SOMEIP_SVC_GW_ROUTING_INST      ((SomeIp_InstanceIdType)0x0001u)
#define SOMEIP_SVC_GW_ROUTING_EG        ((SomeIp_EventGroupIdType)0x0001u)
#define SOMEIP_SVC_GW_ROUTING_MAJOR     ((uint8)1u)
#define SOMEIP_SVC_GW_ROUTING_MINOR     ((uint32)0u)

/* =========================================================================
 * Common SD timing parameters (seconds)
 * ====================================================================== */
#define SOMEIPSD_TTL_DEFAULT            ((uint32)60u)   /* 60 seconds TTL    */
#define SOMEIPSD_TTL_INFINITE           ((uint32)0xFFFFFFu) /* maximum TTL   */

/* =========================================================================
 * Service name lookup (for logging only)
 * ====================================================================== */
static inline const char *Vehicle_ServiceName(SomeIp_ServiceIdType svcId)
{
    switch (svcId)
    {
        case 0x0101u: return "DoorLock";
        case 0x0102u: return "WindowControl";
        case 0x0103u: return "LightControl";
        case 0x0201u: return "EngineStatus";
        case 0x0202u: return "ThrottleControl";
        case 0x0301u: return "LaneKeeping";
        case 0x0302u: return "CollisionWarning";
        case 0x0401u: return "GatewayRouting";
        default:      return "Unknown";
    }
}

#endif /* VEHICLE_SERVICES_H */
