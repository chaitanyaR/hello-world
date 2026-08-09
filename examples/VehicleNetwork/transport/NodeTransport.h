/**
 * NodeTransport.h – UDP Transport Binding for Vehicle Network Nodes
 * AUTOSAR Release R22-11 (Host Simulation Layer)
 *
 * Provides the SoAd/transport integration for SOME/IP SD on the host:
 *   - Binds a UDP socket to the node's SD port
 *   - Implements SomeIpSd_Transmit() to flood SD frames to all peer ports
 *     (simulates the Ethernet switch's multicast forwarding behaviour)
 *   - Runs a background receive thread that feeds frames to the SD module
 *     via SomeIpSd_RxIndication() and dispatches parsed SD events via
 *     application callbacks
 *
 * AUTOSAR mapping:
 *   NodeTransport  ≙  SoAd (Socket Adaptor) + EthIf + EthSwt stubs
 *   SomeIpSd_Transmit  ≙  SoAd_IfTransmit() toward the network
 */

#ifndef NODE_TRANSPORT_H
#define NODE_TRANSPORT_H

#include "Std_Types.h"
#include "SomeIp_Types.h"

/* =========================================================================
 * Configuration
 * ====================================================================== */
#define NODE_TRANSPORT_MAX_PEERS  8u    /* maximum peer nodes per instance */
#define NODE_TRANSPORT_MTU        1500u /* receive buffer size (bytes)     */

/* =========================================================================
 * SD event callbacks
 * Registered by the application to receive parsed SD events.
 * All callbacks are optional (NULL means ignore that event type).
 * ====================================================================== */
typedef struct
{
    /**
     * OnOfferService – a remote node is offering a service.
     *   ttl = 0 means StopOffer.
     */
    void (*OnOfferService)(SomeIp_ServiceIdType  ServiceId,
                           SomeIp_InstanceIdType InstanceId,
                           uint8                 MajorVersion,
                           uint32                MinorVersion,
                           uint32                TtlSeconds);

    /**
     * OnFindService – a remote node is looking for a service.
     */
    void (*OnFindService)(SomeIp_ServiceIdType  ServiceId,
                          SomeIp_InstanceIdType InstanceId,
                          uint8                 MajorVersion,
                          uint32                MinorVersion);

    /**
     * OnSubscribeEventgroup – a remote node is subscribing to an eventgroup.
     *   ttl = 0 means Unsubscribe.
     */
    void (*OnSubscribeEventgroup)(SomeIp_ServiceIdType    ServiceId,
                                  SomeIp_InstanceIdType   InstanceId,
                                  SomeIp_EventGroupIdType EventGroupId,
                                  uint8                   MajorVersion,
                                  uint32                  TtlSeconds);

    /**
     * OnSubscribeEventgroupAck – subscribe ACK received from a provider.
     *   ttl = 0 means NACK (subscribe rejected).
     */
    void (*OnSubscribeEventgroupAck)(SomeIp_ServiceIdType    ServiceId,
                                     SomeIp_InstanceIdType   InstanceId,
                                     SomeIp_EventGroupIdType EventGroupId,
                                     uint8                   MajorVersion,
                                     uint32                  TtlSeconds);
} NodeSdCallbacks;

/* =========================================================================
 * Transport API
 * ====================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NodeTransport_Init
 * Initialises the UDP transport: creates the socket and binds to localPort.
 * Must be called before SomeIpSd_Init().
 *
 * @param localPort   UDP port this node listens on (e.g. NODE_PORT_BCM).
 * @param peerPorts   Array of peer node SD ports (all other nodes).
 * @param peerCount   Number of entries in peerPorts[].
 * @return E_OK / E_NOT_OK.
 */
Std_ReturnType NodeTransport_Init(uint16        localPort,
                                   const uint16 *peerPorts,
                                   uint8         peerCount);

/**
 * NodeTransport_RegisterSdCallbacks
 * Registers application callbacks for parsed SD events.
 * Must be called before NodeTransport_StartReceive().
 *
 * @param callbacks  Pointer to callback struct (copied; can be stack-local).
 */
void NodeTransport_RegisterSdCallbacks(const NodeSdCallbacks *callbacks);

/**
 * NodeTransport_StartReceive
 * Spawns the background receive thread.
 * Must be called after NodeTransport_Init().
 *
 * @return E_OK / E_NOT_OK.
 */
Std_ReturnType NodeTransport_StartReceive(void);

/**
 * NodeTransport_Deinit
 * Stops the receive thread and closes the socket.
 */
void NodeTransport_Deinit(void);

/* SomeIpSd_Transmit is provided by NodeTransport.c and linked to SomeIp_SD.c */

#ifdef __cplusplus
}
#endif

#endif /* NODE_TRANSPORT_H */
