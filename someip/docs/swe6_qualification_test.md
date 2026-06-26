# SWE.6 – Software Qualification Test Specification

**Document ID:** SOMEIP-SWE6-001  
**Version:** 1.0  
**Status:** Released  
**ASPICE Process:** SWE.6 – Software Qualification Testing  
**Parent documents:**  
  - SOMEIP-SWE1-001 (Software Requirements Specification)  
  - SOMEIP-SWE2-001 (Software Architectural Design)  
  - SOMEIP-SWE5-001 (Unit Verification Test Specification)  

---

## 1. Purpose

SWE.6 qualification tests verify that the **integrated SOME/IP software**
satisfies each SWE.1 software requirement. These tests differ from SWE.5 unit
tests in that:

- They exercise the **module as a whole** (not individual functions in isolation).
- They verify **requirement-level behaviours** (correct protocol frames on the wire).
- They provide evidence that the software is ready for system integration (SWS.2 / SIT).

Each test case is traceable to one or more SWE.1 requirements.

---

## 2. Test Environment

| Item | Details |
|------|---------|
| Test framework | Google Test (GTest) v1.14.0 |
| Build | CMake + GTest, same executable as SWE.5 (`someip_tests`) |
| Transport stub | `stub_transport.c` – captures outbound UDP frames |
| CI trigger | `ctest --test-dir build -L qualification` |
| Coverage measurement | Not required for SWE.6 (covered at SWE.5) |

GTest label `qualification` is assigned to each test below.
Add `::testing::Test::RecordProperty("aspice", "swe6")` in test body for tooling.

---

## 3. Qualification Test Cases

### TC-SWE6-001 – SOME/IP Header Structure Compliance
**SWE.1 Ref:** SR-SOMEIP-001, SR-SOMEIP-002  
**Objective:** Verify the serialized frame header exactly matches the AUTOSAR 16-byte layout.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Build a `SomeIp_Message_t` with known field values. | — |
| 2 | Call `SomeIp_Serialize()`. | Returns E_OK. |
| 3 | Inspect bytes [0..15] of the output buffer. | Service ID at [0..1], Method ID at [2..3], Length at [4..7], Client ID at [8..9], Session ID at [10..11], Protocol Ver at [12], Interface Ver at [13], Message Type at [14], Return Code at [15]. |

**Pass criteria:** All bytes at correct offsets with correct big-endian values.

---

### TC-SWE6-002 – Big-Endian Wire Encoding
**SWE.1 Ref:** SR-SOMEIP-002, SR-SOMEIP-105  
**Objective:** Verify multi-byte fields are transmitted in big-endian order on all host architectures.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Set `service_id = 0x1234`, `method_id = 0x5678`. | — |
| 2 | Serialize and inspect raw bytes. | `buf[0]=0x12, buf[1]=0x34, buf[2]=0x56, buf[3]=0x78`. |

---

### TC-SWE6-003 – Protocol Version Fixed at 0x01
**SWE.1 Ref:** SR-SOMEIP-003  
**Objective:** Verify protocol_version field is always 0x01 after serialization.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Create valid message. | — |
| 2 | Serialize. | `buf[12] == 0x01`. |
| 3 | Validate the deserialized header. | `SomeIp_ValidateHeader()` returns E_OK. |

---

### TC-SWE6-004 – Serialization of Payload
**SWE.1 Ref:** SR-SOMEIP-010  
**Objective:** Confirm payload bytes follow the header contiguously and Length field is correct.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Set payload = `{0xCA, 0xFE}`. | — |
| 2 | Serialize. | `out_len == 18`. Length field (`buf[4..7]`) == `0x0000000A` (10 = 8 + 2). |
| 3 | Inspect `buf[16..17]`. | `0xCA, 0xFE`. |

---

### TC-SWE6-005 – Buffer Size Boundary (SR-SOMEIP-011)
**SWE.1 Ref:** SR-SOMEIP-011  
**Objective:** Verify the serializer refuses to overflow a too-small buffer.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Provide `buf_size = 15` (one byte short). | — |
| 2 | Call `SomeIp_Serialize()`. | Returns E_NOT_OK. |
| 3 | Provide `buf_size = 16`. | Returns E_OK. |

---

### TC-SWE6-006 – Deserialization of Valid Frame
**SWE.1 Ref:** SR-SOMEIP-012  
**Objective:** Confirm all header fields are correctly parsed from a canonical wire frame.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Provide the 20-byte canonical frame defined in `test_someip_serializer.cpp`. | — |
| 2 | Call `SomeIp_Deserialize()`. | Returns E_OK. |
| 3 | Check `msg.header.*`. | All fields match original encoding. |

---

### TC-SWE6-007 – Zero-Copy Payload Pointer
**SWE.1 Ref:** SR-SOMEIP-013  
**Objective:** Verify `msg.payload` points into the input buffer (no memcpy).

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Deserialize a frame with payload. | — |
| 2 | Check `msg.payload`. | `msg.payload == &buf[16]` (exact address). |

---

### TC-SWE6-008 – Truncated Frame Rejected
**SWE.1 Ref:** SR-SOMEIP-014  
**Objective:** Confirm malformed / short frames are rejected.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Call `SomeIp_Deserialize()` with `buf_len = 15`. | Returns E_NOT_OK. |
| 2 | Call with `buf_len = 16` but Length field = 100. | Returns E_NOT_OK. |

---

### TC-SWE6-009 – Header Validation: All Fields
**SWE.1 Ref:** SR-SOMEIP-015  
**Objective:** Validate the header validator checks Protocol Version, Length ≥ 8, and Message Type range.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Set `protocol_version = 0x00`. | E_NOT_OK. |
| 2 | Set `length = 7`. | E_NOT_OK. |
| 3 | All fields valid. | E_OK. |

---

### TC-SWE6-010 – Undefined Message Type Rejected
**SWE.1 Ref:** SR-SOMEIP-016  
**Objective:** Verify undeclared message type values fail validation.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Set `message_type = 0x03` (not in spec). | `SomeIp_ValidateHeader()` returns E_NOT_OK. |
| 2 | Set `message_type = 0xFF`. | Returns E_NOT_OK. |
| 3 | Set `message_type = 0x80` (RESPONSE). | Returns E_OK. |

---

### TC-SWE6-011 – OfferService Produces Correct SD Frame
**SWE.1 Ref:** SR-SOMEIP-020  
**Objective:** Verify end-to-end OfferService frame is AUTOSAR-compliant.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Call `SomeIpSd_OfferService(0x1234, 0x0001, 0x01, 1, 60)`. | Transport called once. |
| 2 | Check SOME/IP header in frame. | Service ID=0xFFFF, Method ID=0x8100, MsgType=NOTIFICATION, RC=OK. |
| 3 | Check SD payload. | Entries length=16. Entry type=0x01, SvcID=0x1234, InstID=0x0001, MajVer=0x01, TTL=60, MinVer=1. |

---

### TC-SWE6-012 – StopOffer Service (TTL = 0)
**SWE.1 Ref:** SR-SOMEIP-021  
**Objective:** Verify TTL=0 encodes a Stop Offer Service, not a regular offer.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Call `SomeIpSd_OfferService(svc, inst, major, minor, ttl=0)`. | Returns E_OK. |
| 2 | Inspect TTL bytes in entry. | All three bytes = 0x00. |
| 3 | Entry type still = 0x01. | Offer type does not change for stop. |

---

### TC-SWE6-013 – FindService with Wildcard Parameters
**SWE.1 Ref:** SR-SOMEIP-022  
**Objective:** Verify wildcard parameters (0xFFFF, 0xFF, 0xFFFFFFFF) are encoded verbatim.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Call `SomeIpSd_FindService(0xAAAA, 0xFFFF, 0xFF, 0xFFFFFFFF)`. | Returns E_OK. |
| 2 | Inspect entry bytes. | Type=0x00, SvcID=0xAAAA, InstID=0xFFFF, MajVer=0xFF, MinVer=0xFFFFFFFF. |

---

### TC-SWE6-014 – Subscribe Eventgroup Frame
**SWE.1 Ref:** SR-SOMEIP-023  
**Objective:** Verify SubscribeEventgroup produces a Type-2 SD entry.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Call `SomeIpSd_SubscribeEventgroup(0x0010, 0x0001, 0x0005, 0x01, 30)`. | Returns E_OK. |
| 2 | Inspect frame. | Entry type=0x06, SvcID=0x0010, InstID=0x0001, MajVer=0x01, TTL=30, EventgroupID=0x0005. |

---

### TC-SWE6-015 – Unsubscribe Eventgroup (TTL = 0)
**SWE.1 Ref:** SR-SOMEIP-024  
**Objective:** Verify TTL=0 encodes an Unsubscribe Eventgroup message.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Call `SomeIpSd_SubscribeEventgroup(…, ttl=0)`. | Returns E_OK. |
| 2 | TTL bytes. | 0x00 0x00 0x00. |

---

### TC-SWE6-016 – RxIndication: Valid SD Frame Accepted
**SWE.1 Ref:** SR-SOMEIP-025  
**Objective:** Confirm the receive path processes a well-formed SD frame without error.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Generate SD frame via `SomeIpSd_OfferService()`. | Frame captured in stub. |
| 2 | Pass frame to `SomeIpSd_RxIndication()`. | Returns E_OK. |

---

### TC-SWE6-017 – RxIndication: Non-SD Frame Rejected
**SWE.1 Ref:** SR-SOMEIP-026  
**Objective:** Verify frames destined for non-SD services are rejected.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Create a frame with Service ID = 0x0001 (not SD). | — |
| 2 | Call `SomeIpSd_RxIndication()`. | Returns E_NOT_OK. |

---

### TC-SWE6-018 – Transport Delegation (No Socket Calls in Module)
**SWE.1 Ref:** SR-SOMEIP-030  
**Objective:** Verify all outbound transmission goes through `SomeIpSd_Transmit()`.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Reset stub call count. | count = 0. |
| 2 | Call `SomeIpSd_OfferService()`. | count == 1. |
| 3 | Call `SomeIpSd_FindService()`. | count == 2. |
| 4 | Inspect module source for any `socket()`, `sendto()`, `send()` calls. | None found (static analysis). |

---

### TC-SWE6-019 – No Dynamic Memory Allocation
**SWE.1 Ref:** SR-SOMEIP-100  
**Objective:** Confirm module uses no heap allocations.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Search source files for `malloc`, `calloc`, `realloc`, `free`, `new`, `delete`. | Zero occurrences. |
| 2 | Run under Valgrind or ASAN. | No heap allocations reported from module code. |

---

### TC-SWE6-020 – NULL Pointer Robustness (All Public APIs)
**SWE.1 Ref:** SR-SOMEIP-104  
**Objective:** Every public function must survive NULL pointer arguments without crashing.

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Call each public function with each pointer argument set to NULL in turn. | All return E_NOT_OK. No segfault or undefined behaviour. |

**Functions tested:**
- `SomeIp_Serialize(NULL, …)`, `SomeIp_Serialize(…, NULL, …)`, `SomeIp_Serialize(…, NULL)`
- `SomeIp_Deserialize(NULL, …)`, `SomeIp_Deserialize(…, NULL)`
- `SomeIp_ValidateHeader(NULL)`
- `SomeIpSd_RxIndication(NULL, …)`

---

## 4. Test Execution Summary Template

| TC ID | Title | Result | Date | Tester | Defects |
|-------|-------|--------|------|--------|---------|
| TC-SWE6-001 | Header Structure | — | — | — | — |
| TC-SWE6-002 | Big-Endian Encoding | — | — | — | — |
| TC-SWE6-003 | Protocol Version | — | — | — | — |
| TC-SWE6-004 | Payload Serialization | — | — | — | — |
| TC-SWE6-005 | Buffer Boundary | — | — | — | — |
| TC-SWE6-006 | Deserialization | — | — | — | — |
| TC-SWE6-007 | Zero-Copy Payload | — | — | — | — |
| TC-SWE6-008 | Truncated Frame | — | — | — | — |
| TC-SWE6-009 | Header Validation | — | — | — | — |
| TC-SWE6-010 | Message Type Validation | — | — | — | — |
| TC-SWE6-011 | OfferService Frame | — | — | — | — |
| TC-SWE6-012 | StopOffer TTL=0 | — | — | — | — |
| TC-SWE6-013 | FindService Wildcards | — | — | — | — |
| TC-SWE6-014 | Subscribe Eventgroup | — | — | — | — |
| TC-SWE6-015 | Unsubscribe TTL=0 | — | — | — | — |
| TC-SWE6-016 | RxIndication Accept | — | — | — | — |
| TC-SWE6-017 | RxIndication Reject | — | — | — | — |
| TC-SWE6-018 | Transport Delegation | — | — | — | — |
| TC-SWE6-019 | No Dynamic Memory | — | — | — | — |
| TC-SWE6-020 | NULL Robustness | — | — | — | — |

---

## 5. SWE.6 → SWE.1 Requirement Traceability

| TC ID | SWE.1 Requirement | Coverage |
|-------|-------------------|----------|
| TC-SWE6-001 | SR-SOMEIP-001, SR-SOMEIP-002 | Full |
| TC-SWE6-002 | SR-SOMEIP-002, SR-SOMEIP-105 | Full |
| TC-SWE6-003 | SR-SOMEIP-003 | Full |
| TC-SWE6-004 | SR-SOMEIP-010 | Full |
| TC-SWE6-005 | SR-SOMEIP-011 | Full |
| TC-SWE6-006 | SR-SOMEIP-012 | Full |
| TC-SWE6-007 | SR-SOMEIP-013 | Full |
| TC-SWE6-008 | SR-SOMEIP-014 | Full |
| TC-SWE6-009 | SR-SOMEIP-015 | Full |
| TC-SWE6-010 | SR-SOMEIP-016 | Full |
| TC-SWE6-011 | SR-SOMEIP-020 | Full |
| TC-SWE6-012 | SR-SOMEIP-021 | Full |
| TC-SWE6-013 | SR-SOMEIP-022 | Full |
| TC-SWE6-014 | SR-SOMEIP-023 | Full |
| TC-SWE6-015 | SR-SOMEIP-024 | Full |
| TC-SWE6-016 | SR-SOMEIP-025 | Full |
| TC-SWE6-017 | SR-SOMEIP-026 | Full |
| TC-SWE6-018 | SR-SOMEIP-030 | Full |
| TC-SWE6-019 | SR-SOMEIP-100 | Full |
| TC-SWE6-020 | SR-SOMEIP-104 | Full |

**All 20 SWE.1 functional and non-functional requirements have ≥ 1 qualification test case.**
