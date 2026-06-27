/**
 * ecm_main.c – Engine Control Module (ECM) Node
 * AUTOSAR Release R22-11 – Vehicle Network Example
 *
 * ECU role : Service Provider
 * Switch port : 2 | 100BASE-T1 | 169.254.1.2 (production) / 127.0.0.1:30502 (sim)
 *
 * Services offered:
 *   EngineStatus    (0x0201/0x0001) – engine RPM, temp, run-state notifications
 *   ThrottleControl (0x0202/0x0001) – throttle position commands
 *
 * SD behaviour:
 *   - Offers both services on startup
 *   - Logs SUBSCRIBE messages from ADAS and IPC
 *   - Sends StopOffer before shutdown
 */

#include "Compiler.h"
#include "SomeIp.h"
#include "SomeIp_SD.h"
#include "NodeTransport.h"
#include "VehicleServices.h"

#include <stdio.h>
#include <unistd.h>

/* =========================================================================
 * SD event callbacks
 * ====================================================================== */

static void Ecm_OnOfferService(SomeIp_ServiceIdType  svcId,
                                SomeIp_InstanceIdType instId,
                                uint8                 major,
                                uint32                minor,
                                uint32                ttl)
{
    (void)minor;
    if (ttl == 0u)
    {
        printf("[ECM] <- StopOffer: %s (0x%04X/0x%04X) v%u\n",
               Vehicle_ServiceName(svcId), svcId, instId, major);
    }
    else
    {
        printf("[ECM] <- Offer: %s (0x%04X/0x%04X) v%u TTL=%us\n",
               Vehicle_ServiceName(svcId), svcId, instId, major, (unsigned)ttl);
    }
}

static void Ecm_OnFindService(SomeIp_ServiceIdType  svcId,
                               SomeIp_InstanceIdType instId,
                               uint8                 major,
                               uint32                minor)
{
    (void)minor;
    printf("[ECM] <- Find: %s (0x%04X/0x%04X) v%u\n",
           Vehicle_ServiceName(svcId), svcId, instId, major);

    /* Re-offer if the FIND matches our services */
    if ((svcId == SOMEIP_SVC_ENGINE_STATUS) ||
        (svcId == SOMEIP_SVC_THROTTLE_CTRL))
    {
        printf("[ECM] -> Re-offering %s in response to FIND\n",
               Vehicle_ServiceName(svcId));
        (void)SomeIpSd_OfferService(svcId, instId, major, 0u,
                                    SOMEIPSD_TTL_DEFAULT);
    }
}

static void Ecm_OnSubscribeEventgroup(SomeIp_ServiceIdType    svcId,
                                       SomeIp_InstanceIdType   instId,
                                       SomeIp_EventGroupIdType egId,
                                       uint8                   major,
                                       uint32                  ttl)
{
    (void)major;
    if (ttl == 0u)
    {
        printf("[ECM] <- Unsubscribe: %s (0x%04X/0x%04X) EG=0x%04X\n",
               Vehicle_ServiceName(svcId), svcId, instId, egId);
    }
    else
    {
        printf("[ECM] <- Subscribe: %s (0x%04X/0x%04X) EG=0x%04X TTL=%us\n",
               Vehicle_ServiceName(svcId), svcId, instId, egId, (unsigned)ttl);
        printf("[ECM]    Subscriber added – will send EngineStatus events\n");

        /* In production: validate, add to subscriber list, send SubscribeAck */
    }
}

/* =========================================================================
 * main
 * ====================================================================== */

int main(void)
{
    const uint16 peers[] = {
        NODE_PORT_BCM, NODE_PORT_ADAS, NODE_PORT_GATEWAY, NODE_PORT_IPC
    };
    NodeSdCallbacks cbs = {
        .OnOfferService          = Ecm_OnOfferService,
        .OnFindService           = Ecm_OnFindService,
        .OnSubscribeEventgroup   = Ecm_OnSubscribeEventgroup,
        .OnSubscribeEventgroupAck = NULL
    };

    printf("==========================================================\n");
    printf(" ECM (Engine Control Module) – SOME/IP Node\n");
    printf(" ECU ID: 0x%02X  |  SD Port: %u\n", NODE_ID_ECM, NODE_PORT_ECM);
    printf("==========================================================\n\n");

    /* --- BSW init -------------------------------------------------------- */
    SomeIp_Init(NULL_PTR);
    SomeIpSd_Init(NULL_PTR);

    /* --- Transport init -------------------------------------------------- */
    if (NodeTransport_Init(NODE_PORT_ECM, peers,
                           (uint8)(sizeof(peers) / sizeof(peers[0]))) != E_OK)
    {
        printf("[ECM] ERROR: transport init failed\n");
        return 1;
    }
    NodeTransport_RegisterSdCallbacks(&cbs);
    if (NodeTransport_StartReceive() != E_OK)
    {
        printf("[ECM] ERROR: receive thread start failed\n");
        return 1;
    }

    /* --- Allow other nodes to start (500 ms) ----------------------------- */
    usleep(500000u);

    /* --- Phase 1: Offer services ----------------------------------------- */
    printf("\n[ECM] Phase 1: Offering powertrain services...\n");

    (void)SomeIpSd_OfferService(SOMEIP_SVC_ENGINE_STATUS, SOMEIP_SVC_ENGINE_STATUS_INST,
                                 SOMEIP_SVC_ENGINE_STATUS_MAJOR, SOMEIP_SVC_ENGINE_STATUS_MINOR,
                                 SOMEIPSD_TTL_DEFAULT);
    printf("[ECM] -> OfferService: EngineStatus (0x%04X)\n",
           SOMEIP_SVC_ENGINE_STATUS);

    (void)SomeIpSd_OfferService(SOMEIP_SVC_THROTTLE_CTRL, SOMEIP_SVC_THROTTLE_CTRL_INST,
                                 SOMEIP_SVC_THROTTLE_CTRL_MAJOR, SOMEIP_SVC_THROTTLE_CTRL_MINOR,
                                 SOMEIPSD_TTL_DEFAULT);
    printf("[ECM] -> OfferService: ThrottleControl (0x%04X)\n",
           SOMEIP_SVC_THROTTLE_CTRL);

    /* --- Phase 2–3: Run -------------------------------------------------- */
    printf("\n[ECM] Listening for SD events (7 s)...\n");
    sleep(7u);

    /* --- Shutdown -------------------------------------------------------- */
    printf("\n[ECM] Phase 4: StopOffer all services...\n");

    (void)SomeIpSd_StopOfferService(SOMEIP_SVC_ENGINE_STATUS, SOMEIP_SVC_ENGINE_STATUS_INST,
                                     SOMEIP_SVC_ENGINE_STATUS_MAJOR, SOMEIP_SVC_ENGINE_STATUS_MINOR);
    printf("[ECM] -> StopOfferService: EngineStatus\n");

    (void)SomeIpSd_StopOfferService(SOMEIP_SVC_THROTTLE_CTRL, SOMEIP_SVC_THROTTLE_CTRL_INST,
                                     SOMEIP_SVC_THROTTLE_CTRL_MAJOR, SOMEIP_SVC_THROTTLE_CTRL_MINOR);
    printf("[ECM] -> StopOfferService: ThrottleControl\n");

    usleep(200000u);
    NodeTransport_Deinit();

    printf("\n[ECM] Shutdown complete.\n");
    return 0;
}
