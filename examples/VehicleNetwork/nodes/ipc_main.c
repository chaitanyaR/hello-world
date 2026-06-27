/**
 * ipc_main.c – Instrument Cluster (IPC) Node
 * AUTOSAR Release R22-11 – Vehicle Network Example
 *
 * ECU role : Pure Service Consumer (HMI display)
 * Switch port : 5 | 100BASE-T1 | 169.254.1.5 (production) / 127.0.0.1:30505 (sim)
 *
 * Services consumed (SD Find + EventGroup Subscribe):
 *   EngineStatus  (0x0201/0x0001) from ECM  – display RPM/temp/state
 *   DoorLock      (0x0101/0x0001) from BCM  – display door status
 *   LaneKeeping   (0x0301/0x0001) from ADAS – display ADAS status
 *
 * SD behaviour:
 *   1. Find all three required services
 *   2. When OFFER is received for each, subscribe to its eventgroup
 *   3. "Display" subscribed services on the cluster (log to stdout)
 *   4. StopSubscribe before shutdown
 *
 * IPC does NOT offer any services (pure consumer node).
 */

#include "Compiler.h"
#include "SomeIp.h"
#include "SomeIp_SD.h"
#include "NodeTransport.h"
#include "VehicleServices.h"

#include <stdio.h>
#include "Platform.h"

/* Flags: set when a service has been found and subscribed */
static volatile int Ipc_EngineSubscribed = 0;
static volatile int Ipc_DoorSubscribed   = 0;
static volatile int Ipc_LaneSubscribed   = 0;

/* =========================================================================
 * "Display" helpers – simulate what the cluster would show
 * ====================================================================== */

static void Ipc_DisplayUpdate(const char *widget, const char *value)
{
    printf("[IPC] ┌─ Cluster Display Update ─────────────────┐\n");
    printf("[IPC] │  %-20s : %-18s │\n", widget, value);
    printf("[IPC] └─────────────────────────────────────────────┘\n");
}

/* =========================================================================
 * SD event callbacks
 * ====================================================================== */

static void Ipc_OnOfferService(SomeIp_ServiceIdType  svcId,
                                SomeIp_InstanceIdType instId,
                                uint8                 major,
                                uint32                minor,
                                uint32                ttl)
{
    (void)minor;

    if (ttl == 0u)
    {
        printf("[IPC] <- StopOffer: %s (0x%04X/0x%04X)\n",
               Vehicle_ServiceName(svcId), svcId, instId);

        /* Update display to show service lost */
        if (svcId == SOMEIP_SVC_ENGINE_STATUS)
        {
            Ipc_EngineSubscribed = 0;
            Ipc_DisplayUpdate("EngineStatus", "UNAVAILABLE");
        }
        else if (svcId == SOMEIP_SVC_DOOR_LOCK)
        {
            Ipc_DoorSubscribed = 0;
            Ipc_DisplayUpdate("DoorLock", "UNAVAILABLE");
        }
        else if (svcId == SOMEIP_SVC_LANE_KEEPING)
        {
            Ipc_LaneSubscribed = 0;
            Ipc_DisplayUpdate("LaneKeeping", "UNAVAILABLE");
        }
        return;
    }

    printf("[IPC] <- Offer: %s (0x%04X/0x%04X) v%u TTL=%us\n",
           Vehicle_ServiceName(svcId), svcId, instId, major, (unsigned)ttl);

    /* Subscribe to required services when their OFFER arrives */
    if ((svcId == SOMEIP_SVC_ENGINE_STATUS) && (Ipc_EngineSubscribed == 0))
    {
        Ipc_EngineSubscribed = 1;
        printf("[IPC] -> SubscribeEventgroup: EngineStatus EG=0x%04X\n",
               SOMEIP_SVC_ENGINE_STATUS_EG);
        (void)SomeIpSd_SubscribeEventgroup(
            SOMEIP_SVC_ENGINE_STATUS, SOMEIP_SVC_ENGINE_STATUS_INST,
            SOMEIP_SVC_ENGINE_STATUS_EG, SOMEIP_SVC_ENGINE_STATUS_MAJOR,
            SOMEIPSD_TTL_DEFAULT);
        Ipc_DisplayUpdate("EngineStatus", "SUBSCRIBED");
    }
    else if ((svcId == SOMEIP_SVC_DOOR_LOCK) && (Ipc_DoorSubscribed == 0))
    {
        Ipc_DoorSubscribed = 1;
        printf("[IPC] -> SubscribeEventgroup: DoorLock EG=0x%04X\n",
               SOMEIP_SVC_DOOR_LOCK_EG);
        (void)SomeIpSd_SubscribeEventgroup(
            SOMEIP_SVC_DOOR_LOCK, SOMEIP_SVC_DOOR_LOCK_INST,
            SOMEIP_SVC_DOOR_LOCK_EG, SOMEIP_SVC_DOOR_LOCK_MAJOR,
            SOMEIPSD_TTL_DEFAULT);
        Ipc_DisplayUpdate("DoorLock", "SUBSCRIBED");
    }
    else if ((svcId == SOMEIP_SVC_LANE_KEEPING) && (Ipc_LaneSubscribed == 0))
    {
        Ipc_LaneSubscribed = 1;
        printf("[IPC] -> SubscribeEventgroup: LaneKeeping EG=0x%04X\n",
               SOMEIP_SVC_LANE_KEEPING_EG);
        (void)SomeIpSd_SubscribeEventgroup(
            SOMEIP_SVC_LANE_KEEPING, SOMEIP_SVC_LANE_KEEPING_INST,
            SOMEIP_SVC_LANE_KEEPING_EG, SOMEIP_SVC_LANE_KEEPING_MAJOR,
            SOMEIPSD_TTL_DEFAULT);
        Ipc_DisplayUpdate("LaneKeeping", "SUBSCRIBED");
    }
}

static void Ipc_OnFindService(SomeIp_ServiceIdType  svcId,
                               SomeIp_InstanceIdType instId,
                               uint8                 major,
                               uint32                minor)
{
    (void)minor;
    /* IPC doesn't provide services, but log for diagnostics */
    printf("[IPC] <- Find (passthrough): %s (0x%04X/0x%04X) v%u\n",
           Vehicle_ServiceName(svcId), svcId, instId, major);
}

static void Ipc_OnSubscribeEventgroup(SomeIp_ServiceIdType    svcId,
                                       SomeIp_InstanceIdType   instId,
                                       SomeIp_EventGroupIdType egId,
                                       uint8                   major,
                                       uint32                  ttl)
{
    (void)major;
    /* Log only – IPC doesn't provide event groups */
    printf("[IPC] <- Subscribe (other node): %s (0x%04X/0x%04X) EG=0x%04X TTL=%us\n",
           Vehicle_ServiceName(svcId), svcId, instId, egId, (unsigned)ttl);
}

static void Ipc_OnSubscribeEventgroupAck(SomeIp_ServiceIdType    svcId,
                                          SomeIp_InstanceIdType   instId,
                                          SomeIp_EventGroupIdType egId,
                                          uint8                   major,
                                          uint32                  ttl)
{
    (void)major;
    if (ttl == 0u)
    {
        printf("[IPC] <- SubscribeNack: %s EG=0x%04X – subscription rejected\n",
               Vehicle_ServiceName(svcId), egId);
    }
    else
    {
        printf("[IPC] <- SubscribeAck: %s (0x%04X/0x%04X) EG=0x%04X – active\n",
               Vehicle_ServiceName(svcId), svcId, instId, egId);
        Ipc_DisplayUpdate(Vehicle_ServiceName(svcId), "ACTIVE");
    }
}

/* =========================================================================
 * main
 * ====================================================================== */

int main(void)
{
    const uint16 peers[] = {
        NODE_PORT_BCM, NODE_PORT_ECM, NODE_PORT_ADAS, NODE_PORT_GATEWAY
    };
    NodeSdCallbacks cbs = {
        .OnOfferService           = Ipc_OnOfferService,
        .OnFindService            = Ipc_OnFindService,
        .OnSubscribeEventgroup    = Ipc_OnSubscribeEventgroup,
        .OnSubscribeEventgroupAck = Ipc_OnSubscribeEventgroupAck
    };

    printf("==========================================================\n");
    printf(" IPC (Instrument Cluster) – SOME/IP Node\n");
    printf(" ECU ID: 0x%02X  |  SD Port: %u\n", NODE_ID_IPC, NODE_PORT_IPC);
    printf("==========================================================\n\n");

    /* --- BSW init -------------------------------------------------------- */
    SomeIp_Init(NULL_PTR);
    SomeIpSd_Init(NULL_PTR);

    /* --- Transport init -------------------------------------------------- */
    if (NodeTransport_Init(NODE_PORT_IPC, peers,
                           (uint8)(sizeof(peers) / sizeof(peers[0]))) != E_OK)
    {
        printf("[IPC] ERROR: transport init failed\n");
        return 1;
    }
    NodeTransport_RegisterSdCallbacks(&cbs);
    if (NodeTransport_StartReceive() != E_OK)
    {
        printf("[IPC] ERROR: receive thread start failed\n");
        return 1;
    }

    /* --- Allow other nodes to start (slightly longer delay for IPC) ------ */
    Platform_SleepMs(700u);

    /* --- Phase 2: Find required services --------------------------------- */
    printf("\n[IPC] Phase 2: Finding required vehicle services...\n");

    (void)SomeIpSd_FindService(SOMEIP_SVC_ENGINE_STATUS, SOMEIP_SVC_ENGINE_STATUS_INST,
                                SOMEIP_SVC_ENGINE_STATUS_MAJOR, SOMEIP_SVC_ENGINE_STATUS_MINOR);
    printf("[IPC] -> FindService: EngineStatus (0x%04X)\n", SOMEIP_SVC_ENGINE_STATUS);

    (void)SomeIpSd_FindService(SOMEIP_SVC_DOOR_LOCK, SOMEIP_SVC_DOOR_LOCK_INST,
                                SOMEIP_SVC_DOOR_LOCK_MAJOR, SOMEIP_SVC_DOOR_LOCK_MINOR);
    printf("[IPC] -> FindService: DoorLock (0x%04X)\n", SOMEIP_SVC_DOOR_LOCK);

    (void)SomeIpSd_FindService(SOMEIP_SVC_LANE_KEEPING, SOMEIP_SVC_LANE_KEEPING_INST,
                                SOMEIP_SVC_LANE_KEEPING_MAJOR, SOMEIP_SVC_LANE_KEEPING_MINOR);
    printf("[IPC] -> FindService: LaneKeeping (0x%04X)\n", SOMEIP_SVC_LANE_KEEPING);

    /* --- Phase 3: Run (wait for offers and subscribe) -------------------- */
    printf("\n[IPC] Listening for SD events and subscribing to services (7 s)...\n");
    Platform_SleepMs(7000u);

    /* --- Shutdown: unsubscribe ------------------------------------------- */
    printf("\n[IPC] Phase 4: Stopping subscriptions...\n");

    if (Ipc_EngineSubscribed != 0)
    {
        (void)SomeIpSd_StopSubscribeEventgroup(
            SOMEIP_SVC_ENGINE_STATUS, SOMEIP_SVC_ENGINE_STATUS_INST,
            SOMEIP_SVC_ENGINE_STATUS_EG, SOMEIP_SVC_ENGINE_STATUS_MAJOR);
        printf("[IPC] -> StopSubscribeEventgroup: EngineStatus\n");
    }
    if (Ipc_DoorSubscribed != 0)
    {
        (void)SomeIpSd_StopSubscribeEventgroup(
            SOMEIP_SVC_DOOR_LOCK, SOMEIP_SVC_DOOR_LOCK_INST,
            SOMEIP_SVC_DOOR_LOCK_EG, SOMEIP_SVC_DOOR_LOCK_MAJOR);
        printf("[IPC] -> StopSubscribeEventgroup: DoorLock\n");
    }
    if (Ipc_LaneSubscribed != 0)
    {
        (void)SomeIpSd_StopSubscribeEventgroup(
            SOMEIP_SVC_LANE_KEEPING, SOMEIP_SVC_LANE_KEEPING_INST,
            SOMEIP_SVC_LANE_KEEPING_EG, SOMEIP_SVC_LANE_KEEPING_MAJOR);
        printf("[IPC] -> StopSubscribeEventgroup: LaneKeeping\n");
    }

    Platform_SleepMs(200u);
    NodeTransport_Deinit();

    printf("\n[IPC] Shutdown complete.\n");
    return 0;
}
