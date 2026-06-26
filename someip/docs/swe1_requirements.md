# SWE.1 – Software Requirements Specification

**Document ID:** SOMEIP-SWE1-001  
**Version:** 1.0  
**Status:** Released  
**ASPICE Process:** SWE.1 – Software Requirements Analysis  
**Reference Standards:** AUTOSAR PRS_SOMEIPProtocol R22-11, PRS_SOMEIPServiceDiscovery R22-11  

---

## 1. Purpose

This document captures the software requirements for the SOME/IP middleware module.
Requirements are derived from system-level requirements and the AUTOSAR SOME/IP
Protocol Specification. Each requirement is uniquely identified and shall be
traced to SWE.2 (architecture), SWE.3 (detailed design), SWE.5 (unit tests),
and SWE.6 (qualification tests).

---

## 2. Requirement Notation

| Priority | Keyword |
|----------|---------|
| Mandatory | **shall** |
| Recommended | should |
| Optional | may |

---

## 3. Functional Requirements

### 3.1 Header / Frame Structure

| ID | Requirement | Source |
|----|-------------|--------|
| SR-SOMEIP-001 | The module **shall** implement the 16-byte SOME/IP header as defined in PRS_SOMEIP_00030. | PRS_SOMEIP_00030 |
| SR-SOMEIP-002 | The header **shall** contain Service ID, Method ID, Length, Client ID, Session ID, Protocol Version, Interface Version, Message Type, and Return Code in big-endian byte order. | PRS_SOMEIP_00030 |
| SR-SOMEIP-003 | The Protocol Version field **shall** always be set to 0x01. | PRS_SOMEIP_00052 |

### 3.2 Serialization

| ID | Requirement | Source |
|----|-------------|--------|
| SR-SOMEIP-010 | The module **shall** provide a serialization function that encodes a `SomeIp_Message_t` into a contiguous byte buffer in big-endian wire format. | PRS_SOMEIP_00030 |
| SR-SOMEIP-011 | The serializer **shall** return `E_NOT_OK` if the output buffer is too small to hold the serialized frame. | Defensive programming |
| SR-SOMEIP-012 | The module **shall** provide a deserialization function that parses a raw byte buffer into a `SomeIp_Message_t`. | PRS_SOMEIP_00030 |
| SR-SOMEIP-013 | The deserializer **shall** use zero-copy for payload: the payload pointer **shall** reference the input buffer directly. | Resource constraint |
| SR-SOMEIP-014 | The deserializer **shall** return `E_NOT_OK` if the buffer is shorter than 16 bytes or shorter than `Length + 8` bytes. | PRS_SOMEIP_00030 |

### 3.3 Header Validation

| ID | Requirement | Source |
|----|-------------|--------|
| SR-SOMEIP-015 | The module **shall** provide a header validation function that checks Protocol Version, Length field minimum (≥ 8), and Message Type range. | PRS_SOMEIP_00052, PRS_SOMEIP_00055 |
| SR-SOMEIP-016 | The validator **shall** return `E_NOT_OK` for any undefined Message Type value. | PRS_SOMEIP_00055 |

### 3.4 Service Discovery – Offer / Stop Offer

| ID | Requirement | Source |
|----|-------------|--------|
| SR-SOMEIP-020 | The module **shall** provide `SomeIpSd_OfferService()` to transmit an SD Offer Service entry (Type 0x01) for the given Service ID, Instance ID, major/minor version, and TTL. | PRS_SOMEIPSD_00009 |
| SR-SOMEIP-021 | A TTL value of 0 in `SomeIpSd_OfferService()` **shall** represent a Stop Offer Service message. | PRS_SOMEIPSD_00008 |

### 3.5 Service Discovery – Find Service

| ID | Requirement | Source |
|----|-------------|--------|
| SR-SOMEIP-022 | The module **shall** provide `SomeIpSd_FindService()` to transmit an SD Find Service entry (Type 0x00). Wildcard values (Instance 0xFFFF, Major 0xFF, Minor 0xFFFFFFFF) **shall** be supported. | PRS_SOMEIPSD_00009 |

### 3.6 Service Discovery – Subscribe Eventgroup

| ID | Requirement | Source |
|----|-------------|--------|
| SR-SOMEIP-023 | The module **shall** provide `SomeIpSd_SubscribeEventgroup()` to transmit an SD Subscribe Eventgroup entry (Type 0x06). | PRS_SOMEIPSD_00009 |
| SR-SOMEIP-024 | A TTL value of 0 in `SomeIpSd_SubscribeEventgroup()` **shall** represent an Unsubscribe Eventgroup message. | PRS_SOMEIPSD_00008 |

### 3.7 Service Discovery – Receive

| ID | Requirement | Source |
|----|-------------|--------|
| SR-SOMEIP-025 | The module **shall** provide `SomeIpSd_RxIndication()` to process incoming raw SD frames. | PRS_SOMEIPSD_00001 |
| SR-SOMEIP-026 | `SomeIpSd_RxIndication()` **shall** reject frames whose Service ID ≠ 0xFFFF or Method ID ≠ 0x8100. | PRS_SOMEIPSD_00001 |

### 3.8 Transport Interface

| ID | Requirement | Source |
|----|-------------|--------|
| SR-SOMEIP-030 | The module **shall** delegate all UDP transmission to a platform-provided `SomeIpSd_Transmit()` function; no socket calls **shall** exist within this module. | Portability requirement |

---

## 4. Non-Functional Requirements

| ID | Requirement | Target |
|----|-------------|--------|
| SR-SOMEIP-100 | The module **shall** use no dynamic memory allocation (no malloc/calloc/new). | AUTOSAR memory model |
| SR-SOMEIP-101 | All public functions **shall** return `Std_ReturnType` (E_OK / E_NOT_OK). | AUTOSAR SWS |
| SR-SOMEIP-102 | The implementation **shall** be compilable as C99 without compiler warnings at `-Wall -Wextra`. | Code quality |
| SR-SOMEIP-103 | Maximum stack usage per function call **shall** not exceed 1 024 bytes. | Embedded resource |
| SR-SOMEIP-104 | All public API functions **shall** handle NULL pointer arguments and return `E_NOT_OK`. | Robustness |
| SR-SOMEIP-105 | The implementation **shall** be endian-safe (big-endian wire encoding enforced programmatically). | Portability |

---

## 5. Constraints and Assumptions

- Session IDs are maintained as module-internal static variables; reset on ECU power cycle satisfies PRS_SOMEIPSD_00056.
- SD multicast group (224.0.0.1 port 30490) configuration is the responsibility of the SoAd integration layer.
- SOME/IP-TP (transport protocol for large messages) is scoped out; message types TP_REQUEST etc. are defined but fragmentation logic is not implemented in v1.0.

---

## 6. Requirement Traceability Matrix (RTM)

| Req ID | SWE.2 Element | SWE.3 Unit | SWE.5 Test ID | SWE.6 Test ID |
|--------|---------------|------------|---------------|---------------|
| SR-SOMEIP-001 | SomeIp_Header_t | someip_types.h | SWE5-SER-001 | SWE6-001 |
| SR-SOMEIP-002 | SomeIp_Header_t | someip_serializer.c | SWE5-SER-002 | SWE6-002 |
| SR-SOMEIP-003 | SomeIp_ValidateHeader | someip_serializer.c | SWE5-VAL-001 | SWE6-003 |
| SR-SOMEIP-010 | SomeIp_Serialize | someip_serializer.c | SWE5-SER-003 | SWE6-004 |
| SR-SOMEIP-011 | SomeIp_Serialize | someip_serializer.c | SWE5-SER-004 | SWE6-005 |
| SR-SOMEIP-012 | SomeIp_Deserialize | someip_serializer.c | SWE5-DES-001 | SWE6-006 |
| SR-SOMEIP-013 | SomeIp_Deserialize | someip_serializer.c | SWE5-DES-002 | SWE6-007 |
| SR-SOMEIP-014 | SomeIp_Deserialize | someip_serializer.c | SWE5-DES-003 | SWE6-008 |
| SR-SOMEIP-015 | SomeIp_ValidateHeader | someip_serializer.c | SWE5-VAL-001..003 | SWE6-009 |
| SR-SOMEIP-016 | SomeIp_ValidateHeader | someip_serializer.c | SWE5-VAL-004 | SWE6-010 |
| SR-SOMEIP-020 | SomeIpSd_OfferService | someip_sd.c | SWE5-SD-001 | SWE6-011 |
| SR-SOMEIP-021 | SomeIpSd_OfferService | someip_sd.c | SWE5-SD-002 | SWE6-012 |
| SR-SOMEIP-022 | SomeIpSd_FindService | someip_sd.c | SWE5-SD-003 | SWE6-013 |
| SR-SOMEIP-023 | SomeIpSd_SubscribeEventgroup | someip_sd.c | SWE5-SD-004 | SWE6-014 |
| SR-SOMEIP-024 | SomeIpSd_SubscribeEventgroup | someip_sd.c | SWE5-SD-005 | SWE6-015 |
| SR-SOMEIP-025 | SomeIpSd_RxIndication | someip_sd.c | SWE5-SD-006 | SWE6-016 |
| SR-SOMEIP-026 | SomeIpSd_RxIndication | someip_sd.c | SWE5-SD-007 | SWE6-017 |
| SR-SOMEIP-030 | SomeIpSd_Transmit | someip_sd.c | SWE5-SD-008 | SWE6-018 |
| SR-SOMEIP-100 | All modules | All | SWE5-NF-001 | SWE6-019 |
| SR-SOMEIP-104 | All public API | All | SWE5-NF-002 | SWE6-020 |
