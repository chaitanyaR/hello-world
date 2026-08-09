/**
 * bcm_main.c – Body Control Module (BCM) Node
 * AUTOSAR Release R22-11 – Vehicle Network Example
 *
 * ECU role : Service Provider
 * Switch port : 1 | 100BASE-T1 | 169.254.1.1 (production) / 127.0.0.1:30501 (sim)
 *
 * Services offered:
 *   DoorLock      (0x0101/0x0001)
 *   WindowControl (0x0102/0x0001)
 *   LightControl  (0x0103/0x0001)
 *
 * SD behaviour:
 *   - Offers all three services on startup
 *   - Logs incoming FIND and SUBSCRIBE SD messages from other nodes
 *   - Sends StopOffer before shutdown
 */

#include "Compiler.h"
#include "SomeIp.h"
#include "SomeIp_SD.h"
#include "NodeTransport.h"
#include "VehicleServices.h"

#include <stdio.h>
#include "Platform.h"

#define BCM_TAG   "[BCM  0x%04X/%04X] "

/* =========================================================================
 * SD event callbacks
 * ====================================================================== */

static void Bcm_OnOfferService(SomeIp_ServiceIdType  svcId,
                                SomeIp_InstanceIdType instId,
                                uint8                 major,
                                uint32                minor,
                                uint32                ttl)
{
    (void)minor;
    if (ttl == 0u)
    {
        printf("[BCM] <- StopOffer: %s (0x%04X/0x%04X) v%u\n",
               Vehicle_ServiceName(svcId), svcId, instId, major);
    }
    else
    {
        printf("[BCM] <- Offer: %s (0x%04X/0x%04X) v%u TTL=%us\n",
               Vehicle_ServiceName(svcId), svcId, instId, major, (unsigned)ttl);
    }
}

static void Bcm_OnFindService(SomeIp_ServiceIdType  svcId,
                               SomeIp_InstanceIdType instId,
                               uint8                 major,
                               uint32                minor)
{
    (void)minor;
    printf("[BCM] <- Find: %s (0x%04X/0x%04X) v%u\n",
           Vehicle_ServiceName(svcId), svcId, instId, major);

    /* If the FIND is for one of our services, re-announce it */
    if ((svcId == SOMEIP_SVC_DOOR_LOCK)   ||
        (svcId == SOMEIP_SVC_WINDOW_CTRL) ||
        (svcId == SOMEIP_SVC_LIGHT_CTRL))
    {
        printf("[BCM] -> Re-offering %s in response to FIND\n",
               Vehicle_ServiceName(svcId));
        (void)SomeIpSd_OfferService(svcId, instId, major, 0u,
                                    SOMEIPSD_TTL_DEFAULT);
    }
}

static void Bcm_OnSubscribeEventgroup(SomeIp_ServiceIdType    svcId,
                                       SomeIp_InstanceIdType   instId,
                                       SomeIp_EventGroupIdType egId,
                                       uint8                   major,
                                       uint32                  ttl)
{
    (void)major;
    if (ttl == 0u)
    {
        printf("[BCM] <- Unsubscribe: %s (0x%04X/0x%04X) EG=0x%04X\n",
               Vehicle_ServiceName(svcId), svcId, instId, egId);
    }
    else
    {
        printf("[BCM] <- Subscribe: %s (0x%04X/0x%04X) EG=0x%04X TTL=%us\n",
               Vehicle_ServiceName(svcId), svcId, instId, egId, (unsigned)ttl);

        /* In production: validate subscriber, add to eventgroup list,
         * send SubscribeEventgroupAck. */
    }
}

/* =========================================================================
 * main
 * ====================================================================== */

int main(void)
{
    const uint16 peers[] = {
        NODE_PORT_ECM, NODE_PORT_ADAS, NODE_PORT_GATEWAY, NODE_PORT_IPC
    };
    NodeSdCallbacks cbs = {
        .OnOfferService          = Bcm_OnOfferService,
        .OnFindService           = Bcm_OnFindService,
        .OnSubscribeEventgroup   = Bcm_OnSubscribeEventgroup,
        .OnSubscribeEventgroupAck = NULL
    };

    printf("==========================================================\n");
    printf(" BCM (Body Control Module) – SOME/IP Node\n");
    printf(" ECU ID: 0x%02X  |  SD Port: %u\n", NODE_ID_BCM, NODE_PORT_BCM);
    printf("==========================================================\n\n");

    /* --- BSW init -------------------------------------------------------- */
    SomeIp_Init(NULL_PTR);
    SomeIpSd_Init(NULL_PTR);

    /* --- Transport init -------------------------------------------------- */
    if (NodeTransport_Init(NODE_PORT_BCM, peers,
                           (uint8)(sizeof(peers) / sizeof(peers[0]))) != E_OK)
    {
        printf("[BCM] ERROR: transport init failed\n");
        return 1;
    }
    NodeTransport_RegisterSdCallbacks(&cbs);
    if (NodeTransport_StartReceive() != E_OK)
    {
        printf("[BCM] ERROR: receive thread start failed\n");
        return 1;
    }

    /* --- Allow other nodes to start (500 ms) ----------------------------- */
    Platform_SleepMs(500u);

    /* --- Phase 1: Offer services ----------------------------------------- */
    printf("\n[BCM] Phase 1: Offering body services...\n");

    (void)SomeIpSd_OfferService(SOMEIP_SVC_DOOR_LOCK,   SOMEIP_SVC_DOOR_LOCK_INST,
                                 SOMEIP_SVC_DOOR_LOCK_MAJOR,   SOMEIP_SVC_DOOR_LOCK_MINOR,
                                 SOMEIPSD_TTL_DEFAULT);
    printf("[BCM] -> OfferService: DoorLock (0x%04X)\n", SOMEIP_SVC_DOOR_LOCK);

    (void)SomeIpSd_OfferService(SOMEIP_SVC_WINDOW_CTRL, SOMEIP_SVC_WINDOW_CTRL_INST,
                                 SOMEIP_SVC_WINDOW_CTRL_MAJOR, SOMEIP_SVC_WINDOW_CTRL_MINOR,
                                 SOMEIPSD_TTL_DEFAULT);
    printf("[BCM] -> OfferService: WindowControl (0x%04X)\n", SOMEIP_SVC_WINDOW_CTRL);

    (void)SomeIpSd_OfferService(SOMEIP_SVC_LIGHT_CTRL,  SOMEIP_SVC_LIGHT_CTRL_INST,
                                 SOMEIP_SVC_LIGHT_CTRL_MAJOR,  SOMEIP_SVC_LIGHT_CTRL_MINOR,
                                 SOMEIPSD_TTL_DEFAULT);
    printf("[BCM] -> OfferService: LightControl (0x%04X)\n", SOMEIP_SVC_LIGHT_CTRL);

    /* --- Phase 2–3: Run (listen for SD events) 7 seconds --------------- */
    printf("\n[BCM] Listening for SD events (7 s)...\n");
    Platform_SleepMs(7000u);

    /* --- Shutdown: Stop offering ----------------------------------------- */
    printf("\n[BCM] Phase 4: StopOffer all services...\n");

    (void)SomeIpSd_StopOfferService(SOMEIP_SVC_DOOR_LOCK,   SOMEIP_SVC_DOOR_LOCK_INST,
                                     SOMEIP_SVC_DOOR_LOCK_MAJOR,   SOMEIP_SVC_DOOR_LOCK_MINOR);
    printf("[BCM] -> StopOfferService: DoorLock\n");

    (void)SomeIpSd_StopOfferService(SOMEIP_SVC_WINDOW_CTRL, SOMEIP_SVC_WINDOW_CTRL_INST,
                                     SOMEIP_SVC_WINDOW_CTRL_MAJOR, SOMEIP_SVC_WINDOW_CTRL_MINOR);
    printf("[BCM] -> StopOfferService: WindowControl\n");

    (void)SomeIpSd_StopOfferService(SOMEIP_SVC_LIGHT_CTRL,  SOMEIP_SVC_LIGHT_CTRL_INST,
                                     SOMEIP_SVC_LIGHT_CTRL_MAJOR,  SOMEIP_SVC_LIGHT_CTRL_MINOR);
    printf("[BCM] -> StopOfferService: LightControl\n");

    Platform_SleepMs(200u);
    NodeTransport_Deinit();

    printf("\n[BCM] Shutdown complete.\n");
    return 0;
}
