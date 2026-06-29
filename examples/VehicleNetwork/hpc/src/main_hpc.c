/**
 * main_hpc.c  –  SOVD HPC Node Entry Point
 *
 * High-Performance Computer (HPC) node in the vehicle SOME/IP network.
 * This node acts as:
 *   - SOME/IP service consumer: subscribes to ECM, ADAS, and BCM events
 *   - SOVD provider: exposes all ECU data via HTTP REST on port 8080
 *   - Diagnostic bridge: aggregates data from powertrain (ECM/TCU) and
 *     ADAS domains and makes it accessible to external diagnostic clients
 *
 * Network topology:
 *   HPC port  30506 – receives SOME/IP notifications
 *   HTTP port  8080 – SOVD REST interface + dashboard
 *
 * Consumed services:
 *   0x0201 EngineStatus   (ECM  – powertrain domain)
 *   0x0301 LaneKeeping    (ADAS – HPC domain)
 *   0x0302 CollisionWarn  (ADAS – HPC domain)
 *   0x0101 DoorLock       (BCM  – body domain)
 *   0x0401 GatewayRouting (GW   – network domain)
 */

#include "NodeTransport.h"
#include "SomeIp.h"
#include "SomeIp_SD.h"
#include "VehicleServices.h"
#include "Sovd_DataStore.h"
#include "Sovd_Server.h"
#include "Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Node port and peers
 * ====================================================================== */
#define HPC_LOCAL_PORT     ((uint16)30506u)
#define HPC_NODE_ID        ((uint8)0x06u)

static const uint16 k_peer_ports[] = {
    NODE_PORT_BCM,
    NODE_PORT_ECM,
    NODE_PORT_ADAS,
    NODE_PORT_GATEWAY,
    NODE_PORT_IPC
};

/* =========================================================================
 * SOME/IP SD callbacks – update the data store when services announce
 * ====================================================================== */
static void OnOfferService(SomeIp_ServiceIdType   svcId,
                            SomeIp_InstanceIdType  instId,
                            uint8 major, uint32 minor, uint32 ttl)
{
    (void)instId; (void)major; (void)minor;
    printf("[HPC] SOME/IP OfferService   : 0x%04X (%s)  TTL=%u\n",
           svcId, Vehicle_ServiceName(svcId), (unsigned)ttl);

    /* Subscribe to eventgroups from services we consume */
    if (svcId == SOMEIP_SVC_ENGINE_STATUS) {
        (void)SomeIpSd_SubscribeEventgroup(
            SOMEIP_SVC_ENGINE_STATUS, SOMEIP_SVC_ENGINE_STATUS_INST,
            SOMEIP_SVC_ENGINE_STATUS_EG, SOMEIP_SVC_ENGINE_STATUS_MAJOR,
            SOMEIPSD_TTL_DEFAULT);
    } else if (svcId == SOMEIP_SVC_LANE_KEEPING) {
        (void)SomeIpSd_SubscribeEventgroup(
            SOMEIP_SVC_LANE_KEEPING, SOMEIP_SVC_LANE_KEEPING_INST,
            SOMEIP_SVC_LANE_KEEPING_EG, SOMEIP_SVC_LANE_KEEPING_MAJOR,
            SOMEIPSD_TTL_DEFAULT);
    } else if (svcId == SOMEIP_SVC_COLLISION_WARN) {
        (void)SomeIpSd_SubscribeEventgroup(
            SOMEIP_SVC_COLLISION_WARN, SOMEIP_SVC_COLLISION_WARN_INST,
            SOMEIP_SVC_COLLISION_WARN_EG, SOMEIP_SVC_COLLISION_WARN_MAJOR,
            SOMEIPSD_TTL_DEFAULT);
    } else if (svcId == SOMEIP_SVC_DOOR_LOCK) {
        (void)SomeIpSd_SubscribeEventgroup(
            SOMEIP_SVC_DOOR_LOCK, SOMEIP_SVC_DOOR_LOCK_INST,
            SOMEIP_SVC_DOOR_LOCK_EG, SOMEIP_SVC_DOOR_LOCK_MAJOR,
            SOMEIPSD_TTL_DEFAULT);
    } else if (svcId == SOMEIP_SVC_GW_ROUTING) {
        (void)SomeIpSd_SubscribeEventgroup(
            SOMEIP_SVC_GW_ROUTING, SOMEIP_SVC_GW_ROUTING_INST,
            SOMEIP_SVC_GW_ROUTING_EG, SOMEIP_SVC_GW_ROUTING_MAJOR,
            SOMEIPSD_TTL_DEFAULT);
    }
}

static void OnFindService(SomeIp_ServiceIdType  svcId,
                           SomeIp_InstanceIdType instId,
                           uint8 major, uint32 minor)
{
    (void)svcId; (void)instId; (void)major; (void)minor;
    /* HPC is a pure consumer – no services to offer back */
}

static void OnSubscribeEventgroup(SomeIp_ServiceIdType    svcId,
                                   SomeIp_InstanceIdType   instId,
                                   SomeIp_EventGroupIdType egId,
                                   uint8 major, uint32 ttl)
{
    (void)svcId; (void)instId; (void)egId; (void)major; (void)ttl;
}

static void OnSubscribeEventgroupAck(SomeIp_ServiceIdType    svcId,
                                      SomeIp_InstanceIdType   instId,
                                      SomeIp_EventGroupIdType egId,
                                      uint8 major, uint32 ttl)
{
    (void)instId; (void)egId; (void)major;
    if (ttl > 0u) {
        printf("[HPC] Subscribed to 0x%04X (%s)\n",
               svcId, Vehicle_ServiceName(svcId));
    } else {
        printf("[HPC] Subscription rejected for 0x%04X (%s)\n",
               svcId, Vehicle_ServiceName(svcId));
    }
}

/* =========================================================================
 * Banner
 * ====================================================================== */
static void print_banner(void)
{
    printf("\n");
    printf("============================================================\n");
    printf(" AUTOSAR SOME/IP  –  SOVD HPC Node\n");
    printf(" ASAM SOVD 1.0 | ISO 14229-1 | AUTOSAR R22-11\n");
    printf("============================================================\n");
    printf(" SOME/IP port  : UDP %u (peer of BCM/ECM/ADAS/GW/IPC)\n",
           (unsigned)HPC_LOCAL_PORT);
    printf(" SOVD HTTP     : TCP 8080\n");
    printf(" Dashboard     : http://localhost:8080/dashboard\n");
    printf(" API           : http://localhost:8080/sovd/v1/\n");
    printf(" Domains       : HPC(ADAS) | Powertrain(ECM,TCU) | Body(BCM)\n");
    printf("============================================================\n\n");
}

/* =========================================================================
 * Main
 * ====================================================================== */
int main(void)
{
    NodeSdCallbacks        sd_cbs;
    Sovd_ServerConfigType  srv_cfg;
    uint32                 loop_count = 0u;

    print_banner();

    /* 1. Platform network init (WSAStartup on Windows, no-op on POSIX) */
    if (Platform_NetworkInit() != 0) {
        fprintf(stderr, "[HPC] Platform_NetworkInit failed\n");
        return 1;
    }

    /* 2. Data store – populates defaults + initial fault catalogue */
    Sovd_DataStore_Init();

    /* 3. SOVD HTTP server – binds TCP port 8080, starts accept thread */
    srv_cfg.httpPort       = SOVD_DEFAULT_HTTP_PORT;
    srv_cfg.maxPendingConn = SOVD_DEFAULT_BACKLOG;

    if (Sovd_Server_Init(&srv_cfg) != E_OK) {
        fprintf(stderr, "[HPC] SOVD server init failed\n");
        Sovd_DataStore_Deinit();
        Platform_NetworkDeinit();
        return 1;
    }

    /* 4. SOME/IP transport – UDP port 30506, peers: all 5 ECU nodes */
    if (NodeTransport_Init(HPC_LOCAL_PORT,
                           k_peer_ports,
                           (uint8)(sizeof(k_peer_ports) / sizeof(k_peer_ports[0]))) != E_OK) {
        fprintf(stderr, "[HPC] NodeTransport_Init failed\n");
        Sovd_Server_Deinit();
        Sovd_DataStore_Deinit();
        Platform_NetworkDeinit();
        return 1;
    }

    /* 5. Register SD callbacks */
    (void)memset(&sd_cbs, 0, sizeof(sd_cbs));
    sd_cbs.OnOfferService          = OnOfferService;
    sd_cbs.OnFindService           = OnFindService;
    sd_cbs.OnSubscribeEventgroup   = OnSubscribeEventgroup;
    sd_cbs.OnSubscribeEventgroupAck = OnSubscribeEventgroupAck;
    NodeTransport_RegisterSdCallbacks(&sd_cbs);

    /* 6. SOME/IP + SD init */
    SomeIp_Init(NULL);
    SomeIpSd_Init(NULL);

    /* 7. Find services: ECM engine status, ADAS lane/collision, BCM doors */
    (void)SomeIpSd_FindService(SOMEIP_SVC_ENGINE_STATUS,
                               SOMEIP_SVC_ENGINE_STATUS_INST,
                               SOMEIP_SVC_ENGINE_STATUS_MAJOR,
                               SOMEIP_SVC_ENGINE_STATUS_MINOR);

    (void)SomeIpSd_FindService(SOMEIP_SVC_LANE_KEEPING,
                               SOMEIP_SVC_LANE_KEEPING_INST,
                               SOMEIP_SVC_LANE_KEEPING_MAJOR,
                               SOMEIP_SVC_LANE_KEEPING_MINOR);

    (void)SomeIpSd_FindService(SOMEIP_SVC_COLLISION_WARN,
                               SOMEIP_SVC_COLLISION_WARN_INST,
                               SOMEIP_SVC_COLLISION_WARN_MAJOR,
                               SOMEIP_SVC_COLLISION_WARN_MINOR);

    (void)SomeIpSd_FindService(SOMEIP_SVC_DOOR_LOCK,
                               SOMEIP_SVC_DOOR_LOCK_INST,
                               SOMEIP_SVC_DOOR_LOCK_MAJOR,
                               SOMEIP_SVC_DOOR_LOCK_MINOR);

    (void)SomeIpSd_FindService(SOMEIP_SVC_GW_ROUTING,
                               SOMEIP_SVC_GW_ROUTING_INST,
                               SOMEIP_SVC_GW_ROUTING_MAJOR,
                               SOMEIP_SVC_GW_ROUTING_MINOR);

    /* 8. Start SOME/IP receive thread */
    if (NodeTransport_StartReceive() != E_OK) {
        fprintf(stderr, "[HPC] NodeTransport_StartReceive failed\n");
    }

    printf("[HPC] Running.  Open http://localhost:8080/dashboard in browser.\n\n");

    /* 9. Main loop – advance simulation + drive SOME/IP state machines */
    while (1) {
        SomeIp_MainFunction();
        SomeIpSd_MainFunction();

        /* Advance data simulation every 100 ms */
        Sovd_DataStore_Tick();

        loop_count++;

        /* Periodic heartbeat every 10 s */
        if ((loop_count % 100u) == 0u) {
            printf("[HPC] Tick %5u | Dashboard: http://localhost:8080/dashboard\n",
                   (unsigned)loop_count);
        }

        Platform_SleepMs(100u);
    }

    /* Shutdown (unreachable in this demo but correct in production) */
    NodeTransport_Deinit();
    Sovd_Server_Deinit();
    Sovd_DataStore_Deinit();
    Platform_NetworkDeinit();
    return 0;
}
