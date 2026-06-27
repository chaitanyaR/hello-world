/**
 * gateway_main.c – Domain Gateway Node
 * AUTOSAR Release R22-11 – Vehicle Network Example
 *
 * ECU role : Service Provider + Cross-domain Monitor / Proxy
 * Switch port : 4 | 100BASE-T1 | 169.254.1.4 (production) / 127.0.0.1:30504 (sim)
 *
 * Services offered:
 *   GatewayRouting (0x0401/0x0001) – cross-domain service proxy
 *
 * Gateway function:
 *   - Receives ALL SD messages from the switch (monitor role)
 *   - Builds and maintains a local service routing table
 *   - Logs every SD event for network diagnostics
 *   - In production: bridges services between backbone and body/powertrain domains
 *
 * SD behaviour:
 *   - Finds all known services (to populate routing table)
 *   - Logs OFFER, FIND, and SUBSCRIBE events from all nodes
 */

#include "Compiler.h"
#include "SomeIp.h"
#include "SomeIp_SD.h"
#include "NodeTransport.h"
#include "VehicleServices.h"

#include <stdio.h>
#include <string.h>
#include "Platform.h"

/* =========================================================================
 * Simple service routing table (up to 16 entries)
 * ====================================================================== */
#define GW_TABLE_SIZE   16u

typedef struct
{
    SomeIp_ServiceIdType   ServiceId;
    SomeIp_InstanceIdType  InstanceId;
    uint8                  MajorVersion;
    uint32                 TtlSeconds;
    boolean                Active;
} GwRouteEntry;

static GwRouteEntry Gw_RouteTable[GW_TABLE_SIZE];
static uint8        Gw_RouteCount = 0u;

static void Gw_RouteTable_AddOrUpdate(SomeIp_ServiceIdType  svcId,
                                       SomeIp_InstanceIdType instId,
                                       uint8                 major,
                                       uint32                ttl)
{
    uint8 i;

    /* Update existing entry */
    for (i = 0u; i < Gw_RouteCount; i++)
    {
        if ((Gw_RouteTable[i].ServiceId  == svcId) &&
            (Gw_RouteTable[i].InstanceId == instId))
        {
            Gw_RouteTable[i].TtlSeconds = ttl;
            Gw_RouteTable[i].Active     = (ttl > 0u) ? TRUE : FALSE;
            return;
        }
    }

    /* Add new entry */
    if (Gw_RouteCount < GW_TABLE_SIZE)
    {
        Gw_RouteTable[Gw_RouteCount].ServiceId   = svcId;
        Gw_RouteTable[Gw_RouteCount].InstanceId  = instId;
        Gw_RouteTable[Gw_RouteCount].MajorVersion = major;
        Gw_RouteTable[Gw_RouteCount].TtlSeconds  = ttl;
        Gw_RouteTable[Gw_RouteCount].Active       = (ttl > 0u) ? TRUE : FALSE;
        Gw_RouteCount++;
    }
}

static void Gw_PrintRouteTable(void)
{
    uint8 i;
    printf("\n[GW] ┌─ Routing Table (%u entries) ───────────────────────────┐\n",
           (unsigned)Gw_RouteCount);
    for (i = 0u; i < Gw_RouteCount; i++)
    {
        printf("[GW] │  %-20s  0x%04X/0x%04X  v%u  TTL=%3us  %s\n",
               Vehicle_ServiceName(Gw_RouteTable[i].ServiceId),
               Gw_RouteTable[i].ServiceId,
               Gw_RouteTable[i].InstanceId,
               Gw_RouteTable[i].MajorVersion,
               (unsigned)Gw_RouteTable[i].TtlSeconds,
               (Gw_RouteTable[i].Active == TRUE) ? "UP  " : "DOWN");
    }
    printf("[GW] └────────────────────────────────────────────────────────┘\n\n");
}

/* =========================================================================
 * SD event callbacks
 * ====================================================================== */

static void Gw_OnOfferService(SomeIp_ServiceIdType  svcId,
                               SomeIp_InstanceIdType instId,
                               uint8                 major,
                               uint32                minor,
                               uint32                ttl)
{
    (void)minor;
    Gw_RouteTable_AddOrUpdate(svcId, instId, major, ttl);

    if (ttl == 0u)
    {
        printf("[GW] [RT] StopOffer: %s (0x%04X/0x%04X)\n",
               Vehicle_ServiceName(svcId), svcId, instId);
    }
    else
    {
        printf("[GW] [RT] Offer:     %s (0x%04X/0x%04X) v%u TTL=%us\n",
               Vehicle_ServiceName(svcId), svcId, instId, major, (unsigned)ttl);
    }
}

static void Gw_OnFindService(SomeIp_ServiceIdType  svcId,
                              SomeIp_InstanceIdType instId,
                              uint8                 major,
                              uint32                minor)
{
    (void)minor;
    printf("[GW] [SD] Find:      %s (0x%04X/0x%04X) v%u\n",
           Vehicle_ServiceName(svcId), svcId, instId, major);
}

static void Gw_OnSubscribeEventgroup(SomeIp_ServiceIdType    svcId,
                                      SomeIp_InstanceIdType   instId,
                                      SomeIp_EventGroupIdType egId,
                                      uint8                   major,
                                      uint32                  ttl)
{
    (void)major;
    if (ttl == 0u)
    {
        printf("[GW] [SD] Unsubscribe: %s EG=0x%04X (0x%04X/0x%04X)\n",
               Vehicle_ServiceName(svcId), egId, svcId, instId);
    }
    else
    {
        printf("[GW] [SD] Subscribe:   %s EG=0x%04X (0x%04X/0x%04X) TTL=%us\n",
               Vehicle_ServiceName(svcId), egId, svcId, instId, (unsigned)ttl);
    }
}

/* =========================================================================
 * main
 * ====================================================================== */

int main(void)
{
    const uint16 peers[] = {
        NODE_PORT_BCM, NODE_PORT_ECM, NODE_PORT_ADAS, NODE_PORT_IPC
    };
    NodeSdCallbacks cbs = {
        .OnOfferService           = Gw_OnOfferService,
        .OnFindService            = Gw_OnFindService,
        .OnSubscribeEventgroup    = Gw_OnSubscribeEventgroup,
        .OnSubscribeEventgroupAck = NULL
    };

    printf("==========================================================\n");
    printf(" Gateway (Domain Gateway ECU) – SOME/IP Node\n");
    printf(" ECU ID: 0x%02X  |  SD Port: %u\n", NODE_ID_GATEWAY, NODE_PORT_GATEWAY);
    printf("==========================================================\n\n");

    (void)memset(Gw_RouteTable, 0, sizeof(Gw_RouteTable));

    /* --- BSW init -------------------------------------------------------- */
    SomeIp_Init(NULL_PTR);
    SomeIpSd_Init(NULL_PTR);

    /* --- Transport init -------------------------------------------------- */
    if (NodeTransport_Init(NODE_PORT_GATEWAY, peers,
                           (uint8)(sizeof(peers) / sizeof(peers[0]))) != E_OK)
    {
        printf("[GW] ERROR: transport init failed\n");
        return 1;
    }
    NodeTransport_RegisterSdCallbacks(&cbs);
    if (NodeTransport_StartReceive() != E_OK)
    {
        printf("[GW] ERROR: receive thread start failed\n");
        return 1;
    }

    /* --- Allow other nodes to start (500 ms) ----------------------------- */
    Platform_SleepMs(500u);

    /* --- Phase 1: Offer gateway service ---------------------------------- */
    printf("\n[GW] Phase 1: Offering GatewayRouting service...\n");

    (void)SomeIpSd_OfferService(SOMEIP_SVC_GW_ROUTING, SOMEIP_SVC_GW_ROUTING_INST,
                                 SOMEIP_SVC_GW_ROUTING_MAJOR, SOMEIP_SVC_GW_ROUTING_MINOR,
                                 SOMEIPSD_TTL_DEFAULT);
    printf("[GW] -> OfferService: GatewayRouting (0x%04X)\n", SOMEIP_SVC_GW_ROUTING);

    /* --- Phase 2: Find all known services (populate routing table) ------- */
    Platform_SleepMs(300u);
    printf("\n[GW] Phase 2: Finding all vehicle services...\n");

    (void)SomeIpSd_FindService(SOMEIP_SVC_DOOR_LOCK,   SOMEIP_INSTANCE_ID_ANY,
                                SOMEIP_MAJOR_VERSION_ANY, SOMEIP_MINOR_VERSION_ANY);
    printf("[GW] -> FindService: DoorLock\n");

    (void)SomeIpSd_FindService(SOMEIP_SVC_ENGINE_STATUS, SOMEIP_INSTANCE_ID_ANY,
                                SOMEIP_MAJOR_VERSION_ANY, SOMEIP_MINOR_VERSION_ANY);
    printf("[GW] -> FindService: EngineStatus\n");

    (void)SomeIpSd_FindService(SOMEIP_SVC_LANE_KEEPING, SOMEIP_INSTANCE_ID_ANY,
                                SOMEIP_MAJOR_VERSION_ANY, SOMEIP_MINOR_VERSION_ANY);
    printf("[GW] -> FindService: LaneKeeping\n");

    /* --- Run and monitor SD traffic ------------------------------------- */
    printf("\n[GW] Monitoring SD traffic (5 s)...\n");
    Platform_SleepMs(5000u);

    /* Print routing table snapshot */
    Gw_PrintRouteTable();
    Platform_SleepMs(2000u);

    /* --- Shutdown -------------------------------------------------------- */
    printf("\n[GW] Phase 4: StopOffer GatewayRouting...\n");

    (void)SomeIpSd_StopOfferService(SOMEIP_SVC_GW_ROUTING, SOMEIP_SVC_GW_ROUTING_INST,
                                     SOMEIP_SVC_GW_ROUTING_MAJOR, SOMEIP_SVC_GW_ROUTING_MINOR);
    printf("[GW] -> StopOfferService: GatewayRouting\n");

    Platform_SleepMs(200u);
    NodeTransport_Deinit();

    printf("\n[GW] Shutdown complete.\n");
    return 0;
}
