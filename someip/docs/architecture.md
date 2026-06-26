# SOME/IP AUTOSAR Architecture

**Document ID:** SOMEIP-ARCH-001  
**Version:** 1.0  
**Status:** Released  
**ASPICE Relevant Process:** SWE.2 – Software Architectural Design  

---

## 1. Scope

This document defines the software architecture of the SOME/IP middleware stack
implemented in accordance with:

- AUTOSAR PRS_SOMEIPProtocol R22-11
- AUTOSAR PRS_SOMEIPServiceDiscovery R22-11
- AUTOSAR SWS_SOMEIPTransformer R22-11
- ASPICE PAM 3.1 – SWE.2 Software Architectural Design

---

## 2. Layered Architecture Overview

```mermaid
graph TD
    subgraph ASW["Application Software Layer (ASW)"]
        A1[Service Consumer]
        A2[Service Provider]
        A3[Event Subscriber]
    end

    subgraph SOMEIP["SOME/IP Middleware Layer"]
        direction TB
        SD[Service Discovery\nSomeIpSd]
        RPC[RPC Dispatcher\nSomeIp_Dispatch]
        SER[Serializer / Deserializer\nSomeIp_Serialize / SomeIp_Deserialize]
        VAL[Header Validator\nSomeIp_ValidateHeader]
        SER --> VAL
    end

    subgraph BSW["Basic Software Layer (BSW)"]
        SOAD[Socket Adaptor\nSoAd]
        TCPIP[TCP/IP Stack\nTcpIp]
        ETHIF[Ethernet Interface\nEthIf]
        ETH[Ethernet Driver\nEth]
        SOAD --> TCPIP --> ETHIF --> ETH
    end

    subgraph HW["Hardware"]
        NIC[Ethernet NIC]
    end

    A1 --> RPC
    A2 --> RPC
    A3 --> SD
    SD --> SER
    RPC --> SER
    SER --> SOAD
    ETH --> NIC
```

---

## 3. Component Decomposition (SWE.2 – SW Architecture Elements)

```mermaid
classDiagram
    class SomeIp_Header_t {
        +uint16_t service_id
        +uint16_t method_id
        +uint32_t length
        +uint16_t client_id
        +uint16_t session_id
        +uint8_t  protocol_version
        +uint8_t  interface_version
        +uint8_t  message_type
        +uint8_t  return_code
    }

    class SomeIp_Message_t {
        +SomeIp_Header_t header
        +uint8_t* payload
        +uint32_t payload_length
    }

    class SomeIpSerializer {
        <<module>>
        +SomeIp_Serialize()
        +SomeIp_Deserialize()
        +SomeIp_ValidateHeader()
    }

    class SomeIpSd {
        <<module>>
        +SomeIpSd_OfferService()
        +SomeIpSd_FindService()
        +SomeIpSd_SubscribeEventgroup()
        +SomeIpSd_RxIndication()
        -sd_send_service_entry()
        -session_id : uint16_t
    }

    class SomeIpTransport {
        <<interface>>
        +SomeIpSd_Transmit()
    }

    SomeIp_Message_t *-- SomeIp_Header_t
    SomeIpSerializer ..> SomeIp_Message_t : uses
    SomeIpSd ..> SomeIpSerializer : uses
    SomeIpSd ..> SomeIpTransport : calls
```

---

## 4. Message Sequence – Service Offer / Find / Subscribe

```mermaid
sequenceDiagram
    participant Provider as Service Provider (ASW)
    participant SD as SomeIpSd
    participant Ser as SomeIp Serializer
    participant Net as SoAd / UDP
    participant Consumer as Service Consumer (ASW)

    Note over Provider,Net: Offer Service Flow
    Provider->>SD: SomeIpSd_OfferService(svc_id, inst_id, major, minor, ttl)
    SD->>Ser: SomeIp_Serialize(SD_OFFER msg)
    Ser-->>SD: frame bytes
    SD->>Net: SomeIpSd_Transmit(frame)
    Net-->>Consumer: UDP multicast 224.0.0.1:30490

    Note over Consumer,Net: Find Service Flow
    Consumer->>SD: SomeIpSd_FindService(svc_id, inst_id, 0xFF, 0xFFFFFFFF)
    SD->>Ser: SomeIp_Serialize(SD_FIND msg)
    Ser-->>SD: frame bytes
    SD->>Net: SomeIpSd_Transmit(frame)

    Note over Consumer,Net: Subscribe Eventgroup Flow
    Consumer->>SD: SomeIpSd_SubscribeEventgroup(svc_id, inst_id, eg_id, major, ttl)
    SD->>Ser: SomeIp_Serialize(SD_SUBSCRIBE msg)
    Ser-->>SD: frame bytes
    SD->>Net: SomeIpSd_Transmit(frame)

    Note over Net,Consumer: Incoming SD Frame
    Net->>SD: SomeIpSd_RxIndication(buf, len)
    SD->>Ser: SomeIp_Deserialize(buf, len)
    Ser-->>SD: SomeIp_Message_t
    SD->>SD: parse entries → dispatch
```

---

## 5. SOME/IP Header Wire Format (PRS_SOMEIP_00030)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          Service ID           |           Method ID           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Length                             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           Client ID           |           Session ID          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Proto Version | Iface Version |  Message Type |  Return Code  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Payload (variable)                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

---

## 6. SD Entry Type-1 Wire Format (PRS_SOMEIPSD_00009)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |  Idx 1st Opt  |  Idx 2nd Opt  |#Opts1 |#Opts2 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          Service ID           |         Instance ID           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Major Version |          TTL (24-bit)                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Minor Version                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

---

## 7. Static Memory Budget

| Module               | .text (approx) | .bss/.data     |
|----------------------|----------------|----------------|
| someip_serializer.c  | ~600 bytes      | 0              |
| someip_sd.c          | ~1 200 bytes    | 4 bytes (2× session_id) |
| **Total**            | **~1 800 bytes**| **4 bytes**    |

Stack usage: ~512 bytes peak (SD frame buffer on stack in `sd_send_service_entry`).

---

## 8. ASPICE SWE.2 Traceability

| Architecture Element     | SWE.1 Requirement IDs                          |
|--------------------------|------------------------------------------------|
| SomeIp_Header_t          | SR-SOMEIP-001, SR-SOMEIP-002, SR-SOMEIP-003    |
| SomeIp_Serialize()       | SR-SOMEIP-010, SR-SOMEIP-011                   |
| SomeIp_Deserialize()     | SR-SOMEIP-012, SR-SOMEIP-013, SR-SOMEIP-014    |
| SomeIp_ValidateHeader()  | SR-SOMEIP-015, SR-SOMEIP-016                   |
| SomeIpSd_OfferService()  | SR-SOMEIP-020, SR-SOMEIP-021                   |
| SomeIpSd_FindService()   | SR-SOMEIP-022                                  |
| SomeIpSd_SubscribeEventgroup() | SR-SOMEIP-023, SR-SOMEIP-024             |
| SomeIpSd_RxIndication()  | SR-SOMEIP-025, SR-SOMEIP-026                   |
| SomeIpSd_Transmit (stub) | SR-SOMEIP-030                                  |
