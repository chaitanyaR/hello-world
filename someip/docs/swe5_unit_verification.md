# SWE.5 – Software Unit Verification Test Specification

**Document ID:** SOMEIP-SWE5-001  
**Version:** 1.0  
**Status:** Released  
**ASPICE Process:** SWE.5 – Software Unit Verification  
**Parent documents:**  
  - SOMEIP-SWE1-001 (Software Requirements Specification)  
  - SOMEIP-SWE2-001 (Software Architectural Design)  

---

## 1. Purpose

This document specifies the unit verification strategy and individual test cases
for the SOME/IP middleware software units. Unit tests are implemented using the
Google Test (GTest) framework and are located in `someip/tests/`.

Per ASPICE SWE.5:
- Each software unit is verified in isolation.
- Coverage criteria: **100% statement**, **100% branch**, **MC/DC** for boolean
  compound conditions.
- Test results are compared against expected outputs defined below.
- Traceability is maintained from test case → software unit → SWE.1 requirement.

---

## 2. Test Environment

| Item | Details |
|------|---------|
| Framework | Google Test (GTest) v1.14.0 |
| Build | CMake 3.16+, C++17, C99 |
| Compiler | GCC ≥ 11 with `-Wall -Wextra -Werror` |
| Coverage | gcov / lcov (HTML report) |
| Test executable | `someip_tests` |
| Stub | `stub_transport.c` – replaces `SomeIpSd_Transmit()` BSW call |
| CI trigger | `ctest --test-dir build` |

---

## 3. Software Unit: `SOMEIP_Serializer` (`someip_serializer.c`)

### 3.1 Test Cases – Serialization

| Test ID | Test Name | Pre-condition | Stimulus | Expected Result | SWE.1 Ref |
|---------|-----------|---------------|----------|-----------------|-----------|
| SWE5-SER-001 | Header-only message size | Valid msg, no payload | `SomeIp_Serialize()` | `out_len == 16`, returns E_OK | SR-SOMEIP-010 |
| SWE5-SER-002 | Big-endian header encoding | Valid msg with known field values | `SomeIp_Serialize()` | Each byte at expected offset matches | SR-SOMEIP-002 |
| SWE5-SER-003 | Payload appended after header | msg with 4-byte payload | `SomeIp_Serialize()` | `out_len == 20`, payload at buf[16..19] | SR-SOMEIP-010 |
| SWE5-SER-004 | Buffer too small | buf_size < 16 + payload | `SomeIp_Serialize()` | Returns E_NOT_OK | SR-SOMEIP-011 |
| SWE5-SER-005 | Exact-fit buffer | buf_size == frame size | `SomeIp_Serialize()` | Returns E_OK | SR-SOMEIP-011 |
| SWE5-SER-006 | NULL msg pointer | msg = NULL | `SomeIp_Serialize()` | Returns E_NOT_OK | SR-SOMEIP-104 |
| SWE5-SER-007 | NULL buf pointer | buf = NULL | `SomeIp_Serialize()` | Returns E_NOT_OK | SR-SOMEIP-104 |
| SWE5-SER-008 | NULL out_len pointer | out_len = NULL | `SomeIp_Serialize()` | Returns E_NOT_OK | SR-SOMEIP-104 |

### 3.2 Test Cases – Deserialization

| Test ID | Test Name | Pre-condition | Stimulus | Expected Result | SWE.1 Ref |
|---------|-----------|---------------|----------|-----------------|-----------|
| SWE5-DES-001 | All header fields parsed | 20-byte valid frame | `SomeIp_Deserialize()` | All `msg.header.*` match expected values | SR-SOMEIP-012 |
| SWE5-DES-002 | Zero-copy payload pointer | 20-byte frame with payload | `SomeIp_Deserialize()` | `msg.payload == &buf[16]` | SR-SOMEIP-013 |
| SWE5-DES-003 | Frame shorter than 16 bytes | buf_len = 15 | `SomeIp_Deserialize()` | Returns E_NOT_OK | SR-SOMEIP-014 |
| SWE5-DES-004 | Length field exceeds buffer | Length claims 100 bytes, buf has 20 | `SomeIp_Deserialize()` | Returns E_NOT_OK | SR-SOMEIP-014 |
| SWE5-DES-005 | NULL buf pointer | buf = NULL | `SomeIp_Deserialize()` | Returns E_NOT_OK | SR-SOMEIP-104 |
| SWE5-DES-006 | NULL msg pointer | msg = NULL | `SomeIp_Deserialize()` | Returns E_NOT_OK | SR-SOMEIP-104 |
| SWE5-DES-007 | Header-only frame (no payload) | Length field = 8 | `SomeIp_Deserialize()` | `payload == NULL`, `payload_length == 0` | SR-SOMEIP-012 |
| SWE5-DES-008 | Round-trip fidelity | Arbitrary valid message | Serialize → Deserialize | All fields identical, payload matches | SR-SOMEIP-010, SR-SOMEIP-012 |

### 3.3 Test Cases – Header Validation

| Test ID | Test Name | Pre-condition | Stimulus | Expected Result | SWE.1 Ref |
|---------|-----------|---------------|----------|-----------------|-----------|
| SWE5-VAL-001 | Valid header | All fields within spec | `SomeIp_ValidateHeader()` | Returns E_OK | SR-SOMEIP-015 |
| SWE5-VAL-002 | Wrong protocol version | protocol_version = 0x02 | `SomeIp_ValidateHeader()` | Returns E_NOT_OK | SR-SOMEIP-003 |
| SWE5-VAL-003 | Length field below minimum | length = 7 | `SomeIp_ValidateHeader()` | Returns E_NOT_OK | SR-SOMEIP-015 |
| SWE5-VAL-004 | All valid message types | Each of 10 defined types | `SomeIp_ValidateHeader()` | Returns E_OK for each | SR-SOMEIP-016 |
| SWE5-VAL-005 | Undefined message types | Types: 0x03, 0x04, 0x05, 0x10, 0xFF | `SomeIp_ValidateHeader()` | Returns E_NOT_OK for each | SR-SOMEIP-016 |
| SWE5-VAL-006 | NULL header pointer | header = NULL | `SomeIp_ValidateHeader()` | Returns E_NOT_OK | SR-SOMEIP-104 |
| SWE5-VAL-007 | Length boundary = 8 | length = 8 | `SomeIp_ValidateHeader()` | Returns E_OK | SR-SOMEIP-015 |

---

## 4. Software Unit: `SOMEIP_SD` (`someip_sd.c`)

| Test ID | Test Name | Pre-condition | Stimulus | Expected Result | SWE.1 Ref |
|---------|-----------|---------------|----------|-----------------|-----------|
| SWE5-SD-001 | OfferService frame structure | Transport stub ready | `SomeIpSd_OfferService(0x1234, 0x0001, 0x01, 1, 3)` | Frame: SD SvcID=0xFFFF, MethID=0x8100, entry type=0x01, correct SvcID/InstID/TTL/MinorVer | SR-SOMEIP-020 |
| SWE5-SD-002 | StopOffer TTL=0 | Transport stub ready | `SomeIpSd_OfferService(…, ttl=0)` | TTL bytes in entry = 0x00 0x00 0x00 | SR-SOMEIP-021 |
| SWE5-SD-003 | FindService entry type 0x00 | Transport stub ready | `SomeIpSd_FindService(0xAAAA, 0xFFFF, 0xFF, 0xFFFFFFFF)` | Entry type = 0x00, wildcard values encoded correctly | SR-SOMEIP-022 |
| SWE5-SD-004 | SubscribeEventgroup entry type 0x06 | Transport stub ready | `SomeIpSd_SubscribeEventgroup(…)` | Entry type = 0x06, eventgroup_id correct | SR-SOMEIP-023 |
| SWE5-SD-005 | UnsubscribeEventgroup TTL=0 | Transport stub ready | `SomeIpSd_SubscribeEventgroup(…, ttl=0)` | TTL bytes in entry = 0x00 0x00 0x00 | SR-SOMEIP-024 |
| SWE5-SD-006 | RxIndication accepts valid SD frame | Valid SD frame (round-tripped) | `SomeIpSd_RxIndication(frame, len)` | Returns E_OK | SR-SOMEIP-025 |
| SWE5-SD-007 | RxIndication rejects non-SD frame | Frame with SvcID ≠ 0xFFFF | `SomeIpSd_RxIndication(frame, len)` | Returns E_NOT_OK | SR-SOMEIP-026 |
| SWE5-SD-008 | Transport failure propagated | Stub returns E_NOT_OK | `SomeIpSd_OfferService(…)` | Returns E_NOT_OK | SR-SOMEIP-030 |
| SWE5-SD-009 | RxIndication NULL buffer | buf = NULL | `SomeIpSd_RxIndication(NULL, 32)` | Returns E_NOT_OK | SR-SOMEIP-104 |
| SWE5-SD-010 | RxIndication payload too short | SD frame with length=8 (no SD payload) | `SomeIpSd_RxIndication(frame, 16)` | Returns E_NOT_OK | SR-SOMEIP-025 |
| SWE5-SD-011 | Session ID increments each send | Two consecutive sends | `SomeIpSd_OfferService()` called twice | sess_id_2 == sess_id_1 + 1 | PRS_SOMEIPSD_00056 |
| SWE5-SD-012 | Reboot flag set in SD flags | Fresh transport stub | `SomeIpSd_OfferService(…)` | Bit 7 of flags byte = 1 | PRS_SOMEIPSD_00049 |
| SWE5-SD-013 | Options array length = 0 | Transport stub | `SomeIpSd_OfferService(…)` | 4-byte options length field after entries = 0 | AUTOSAR SD |

---

## 5. Non-Functional Tests

| Test ID | Test Name | Pre-condition | Stimulus | Expected Result | SWE.1 Ref |
|---------|-----------|---------------|----------|-----------------|-----------|
| SWE5-NF-001 | NULL safety all public APIs | — | Pass NULL to every argument of every public function | All return E_NOT_OK, no crash | SR-SOMEIP-104 |
| SWE5-NF-002 | Frame size = 16 + payload_length | Various payload sizes | `SomeIp_Serialize()` with plen ∈ {0,1,4,16,100} | out_len == 16 + plen for each | SR-SOMEIP-103 |

---

## 6. Coverage Goals

| Unit | Statement | Branch | MC/DC |
|------|-----------|--------|-------|
| `someip_serializer.c` | 100% | 100% | 100% |
| `someip_sd.c` | 100% | 100% | N/A (no compound boolean) |

---

## 7. Pass / Fail Criteria

| Criterion | Threshold |
|-----------|-----------|
| Test cases passed | 100% (all tests green) |
| Statement coverage | ≥ 100% |
| Branch coverage | ≥ 100% |
| Compiler warnings | 0 at `-Wall -Wextra -Werror` |
| Static analysis defects (Severity High) | 0 |

---

## 8. SWE.5 → SWE.3 Traceability

| Test ID | Software Unit | Source File |
|---------|---------------|-------------|
| SWE5-SER-* | `SomeIp_Serialize`, `SomeIp_Deserialize` | `someip_serializer.c` |
| SWE5-DES-* | `SomeIp_Deserialize` | `someip_serializer.c` |
| SWE5-VAL-* | `SomeIp_ValidateHeader` | `someip_serializer.c` |
| SWE5-SD-* | `SomeIpSd_OfferService`, `SomeIpSd_FindService`, `SomeIpSd_SubscribeEventgroup`, `SomeIpSd_RxIndication`, `sd_send_service_entry` | `someip_sd.c` |
| SWE5-NF-* | All public APIs | `someip_serializer.c`, `someip_sd.c` |
