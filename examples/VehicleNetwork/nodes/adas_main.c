/**
 * adas_main.c – ADAS Controller Node
 * AUTOSAR Release R22-11 – Vehicle Network Example
 *
 * ECU role : Service Provider AND Consumer
 * Switch port : 3 | 100BASE-T1 | 169.254.1.3 (production) / 127.0.0.1:30503 (sim)
 *
 * Services offered:
 *   LaneKeeping      (0x0301/0x0001)
 *   CollisionWarning (0x0302/0x0001)
 *
 * Services consumed (SD Find + EventGroup Subscribe):
 *   EngineStatus (0x0201/0x0001) from ECM
 *     – ADAS needs vehicle speed / engine state for ADAS function enable
 *
 * SD behaviour:
 *   1. Offer LaneKeeping and CollisionWarning
 *   2. Find EngineStatus; when OFFER is received, subscribe to its eventgroup
 *   3. Log all received SD messages
 *   4. StopOffer and StopSubscribe before shutdown
 */

#include "Compiler.h"
#include "SomeIp.h"
#include "SomeIp_SD.h"
#include "NodeTransport.h"
#include "VehicleServices.h"

#include <stdio.h>
#include "Platform.h"

/* Engine service discovered flag (set in OnOffer callback) */
static volatile int Adas_EngineFound = 0;

/* =========================================================================
 * SD event callbacks
 * ====================================================================== */

static void Adas_OnOfferService(SomeIp_ServiceIdType  svcId,
                                 SomeIp_InstanceIdType instId,
                                 uint8                 major,
                                 uint32                minor,
                                 uint32                ttl)
{
    (void)minor;

    if (ttl == 0u)
    {
        printf("[ADAS] <- StopOffer: %s (0x%04X/0x%04X) v%u\n",
               Vehicle_ServiceName(svcId), svcId, instId, major);

        if (svcId == SOMEIP_SVC_ENGINE_STATUS)
        {
            Adas_EngineFound = 0;
            printf("[ADAS]    EngineStatus lost – ADAS functions suspended\n");
        }
        return;
    }

    printf("[ADAS] <- Offer: %s (0x%04X/0x%04X) v%u TTL=%us\n",
           Vehicle_ServiceName(svcId), svcId, instId, major, (unsigned)ttl);

    /* When EngineStatus OFFER is received, subscribe to its eventgroup */
    if ((svcId == SOMEIP_SVC_ENGINE_STATUS) && (Adas_EngineFound == 0))
    {
        Adas_EngineFound = 1;
        printf("[ADAS]    EngineStatus found! Subscribing to eventgroup 0x%04X...\n",
               SOMEIP_SVC_ENGINE_STATUS_EG);

        (void)SomeIpSd_SubscribeEventgroup(
            SOMEIP_SVC_ENGINE_STATUS,
            SOMEIP_SVC_ENGINE_STATUS_INST,
            SOMEIP_SVC_ENGINE_STATUS_EG,
            SOMEIP_SVC_ENGINE_STATUS_MAJOR,
            SOMEIPSD_TTL_DEFAULT);

        printf("[ADAS] -> SubscribeEventgroup: EngineStatus EG=0x%04X\n",
               SOMEIP_SVC_ENGINE_STATUS_EG);
    }
}

static void Adas_OnFindService(SomeIp_ServiceIdType  svcId,
                                SomeIp_InstanceIdType instId,
                                uint8                 major,
                                uint32                minor)
{
    (void)minor;
    printf("[ADAS] <- Find: %s (0x%04X/0x%04X) v%u\n",
           Vehicle_ServiceName(svcId), svcId, instId, major);

    /* Re-offer if FIND is for one of our services */
    if ((svcId == SOMEIP_SVC_LANE_KEEPING) ||
        (svcId == SOMEIP_SVC_COLLISION_WARN))
    {
        printf("[ADAS] -> Re-offering %s\n", Vehicle_ServiceName(svcId));
        (void)SomeIpSd_OfferService(svcId, instId, major, 0u,
                                    SOMEIPSD_TTL_DEFAULT);
    }
}

static void Adas_OnSubscribeEventgroup(SomeIp_ServiceIdType    svcId,
                                        SomeIp_InstanceIdType   instId,
                                        SomeIp_EventGroupIdType egId,
                                        uint8                   major,
                                        uint32                  ttl)
{
    (void)major;
    if (ttl == 0u)
    {
        printf("[ADAS] <- Unsubscribe: %s (0x%04X/0x%04X) EG=0x%04X\n",
               Vehicle_ServiceName(svcId), svcId, instId, egId);
    }
    else
    {
        printf("[ADAS] <- Subscribe: %s (0x%04X/0x%04X) EG=0x%04X TTL=%us\n",
               Vehicle_ServiceName(svcId), svcId, instId, egId, (unsigned)ttl);
    }
}

static void Adas_OnSubscribeEventgroupAck(SomeIp_ServiceIdType    svcId,
                                           SomeIp_InstanceIdType   instId,
                                           SomeIp_EventGroupIdType egId,
                                           uint8                   major,
                                           uint32                  ttl)
{
    (void)major;
    if (ttl == 0u)
    {
        printf("[ADAS] <- SubscribeNack: %s (0x%04X/0x%04X) EG=0x%04X – rejected\n",
               Vehicle_ServiceName(svcId), svcId, instId, egId);
    }
    else
    {
        printf("[ADAS] <- SubscribeAck: %s (0x%04X/0x%04X) EG=0x%04X – accepted\n",
               Vehicle_ServiceName(svcId), svcId, instId, egId);
    }
}

/* =========================================================================
 * main
 * ====================================================================== */

int main(void)
{
    const uint16 peers[] = {
        NODE_PORT_BCM, NODE_PORT_ECM, NODE_PORT_GATEWAY, NODE_PORT_IPC
    };
    NodeSdCallbacks cbs = {
        .OnOfferService           = Adas_OnOfferService,
        .OnFindService            = Adas_OnFindService,
        .OnSubscribeEventgroup    = Adas_OnSubscribeEventgroup,
        .OnSubscribeEventgroupAck = Adas_OnSubscribeEventgroupAck
    };

    printf("==========================================================\n");
    printf(" ADAS (Advanced Driver Assistance) – SOME/IP Node\n");
    printf(" ECU ID: 0x%02X  |  SD Port: %u\n", NODE_ID_ADAS, NODE_PORT_ADAS);
    printf("==========================================================\n\n");

    /* --- BSW init -------------------------------------------------------- */
    SomeIp_Init(NULL_PTR);
    SomeIpSd_Init(NULL_PTR);

    /* --- Transport init -------------------------------------------------- */
    if (NodeTransport_Init(NODE_PORT_ADAS, peers,
                           (uint8)(sizeof(peers) / sizeof(peers[0]))) != E_OK)
    {
        printf("[ADAS] ERROR: transport init failed\n");
        return 1;
    }
    NodeTransport_RegisterSdCallbacks(&cbs);
    if (NodeTransport_StartReceive() != E_OK)
    {
        printf("[ADAS] ERROR: receive thread start failed\n");
        return 1;
    }

    /* --- Allow other nodes to start (500 ms) ----------------------------- */
    Platform_SleepMs(500u);

    /* --- Phase 1: Offer own services ------------------------------------ */
    printf("\n[ADAS] Phase 1: Offering ADAS services...\n");

    (void)SomeIpSd_OfferService(SOMEIP_SVC_LANE_KEEPING, SOMEIP_SVC_LANE_KEEPING_INST,
                                 SOMEIP_SVC_LANE_KEEPING_MAJOR, SOMEIP_SVC_LANE_KEEPING_MINOR,
                                 SOMEIPSD_TTL_DEFAULT);
    printf("[ADAS] -> OfferService: LaneKeeping (0x%04X)\n", SOMEIP_SVC_LANE_KEEPING);

    (void)SomeIpSd_OfferService(SOMEIP_SVC_COLLISION_WARN, SOMEIP_SVC_COLLISION_WARN_INST,
                                 SOMEIP_SVC_COLLISION_WARN_MAJOR, SOMEIP_SVC_COLLISION_WARN_MINOR,
                                 SOMEIPSD_TTL_DEFAULT);
    printf("[ADAS] -> OfferService: CollisionWarning (0x%04X)\n", SOMEIP_SVC_COLLISION_WARN);

    /* --- Phase 2: Find required services --------------------------------- */
    Platform_SleepMs(200u);
    printf("\n[ADAS] Phase 2: Finding EngineStatus (needed for ADAS enable)...\n");

    (void)SomeIpSd_FindService(SOMEIP_SVC_ENGINE_STATUS, SOMEIP_SVC_ENGINE_STATUS_INST,
                                SOMEIP_SVC_ENGINE_STATUS_MAJOR, SOMEIP_SVC_ENGINE_STATUS_MINOR);
    printf("[ADAS] -> FindService: EngineStatus (0x%04X)\n", SOMEIP_SVC_ENGINE_STATUS);

    /* --- Phase 3: Run and process SD events ------------------------------ */
    printf("\n[ADAS] Listening for SD events (7 s)...\n");
    Platform_SleepMs(7000u);

    /* --- Shutdown -------------------------------------------------------- */
    printf("\n[ADAS] Phase 4: StopOffer + StopSubscribe...\n");

    (void)SomeIpSd_StopOfferService(SOMEIP_SVC_LANE_KEEPING, SOMEIP_SVC_LANE_KEEPING_INST,
                                     SOMEIP_SVC_LANE_KEEPING_MAJOR, SOMEIP_SVC_LANE_KEEPING_MINOR);
    printf("[ADAS] -> StopOfferService: LaneKeeping\n");

    (void)SomeIpSd_StopOfferService(SOMEIP_SVC_COLLISION_WARN, SOMEIP_SVC_COLLISION_WARN_INST,
                                     SOMEIP_SVC_COLLISION_WARN_MAJOR, SOMEIP_SVC_COLLISION_WARN_MINOR);
    printf("[ADAS] -> StopOfferService: CollisionWarning\n");

    if (Adas_EngineFound != 0)
    {
        (void)SomeIpSd_StopSubscribeEventgroup(
            SOMEIP_SVC_ENGINE_STATUS, SOMEIP_SVC_ENGINE_STATUS_INST,
            SOMEIP_SVC_ENGINE_STATUS_EG, SOMEIP_SVC_ENGINE_STATUS_MAJOR);
        printf("[ADAS] -> StopSubscribeEventgroup: EngineStatus\n");
    }

    Platform_SleepMs(200u);
    NodeTransport_Deinit();

    printf("\n[ADAS] Shutdown complete.\n");
    return 0;
}
