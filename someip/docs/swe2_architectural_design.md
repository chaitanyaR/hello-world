# SWE.2 – Software Architectural Design

**Document ID:** SOMEIP-SWE2-001  
**Version:** 1.0  
**Status:** Released  
**ASPICE Process:** SWE.2 – Software Architectural Design  
**Parent:** SOMEIP-SWE1-001 (Software Requirements Specification)  

---

## 1. Purpose

This document defines the software architectural design for the SOME/IP module,
decomposing the software into architectural elements (components, interfaces,
data flows) and establishing the framework for detailed design (SWE.3) and
unit verification (SWE.5 / SWE.6).

The architecture targets **ASPICE Level 2** compliance (managed process):
all work products are planned, tracked, and version-controlled.

---

## 2. Architectural Principles

| Principle | Application |
|-----------|-------------|
| Separation of concerns | Serializer and SD are independent modules |
| Dependency inversion | SD depends on abstract `SomeIpSd_Transmit` interface |
| Zero dynamic allocation | All buffers are stack-allocated |
| AUTOSAR layering | Module sits between ASW and SoAd, never calls drivers directly |
| Defensive programming | All public functions validate all pointer arguments |

---

## 3. Module Decomposition

### 3.1 Module List

| Module | File(s) | Responsibility |
|--------|---------|----------------|
| `SOMEIP_Types` | `someip_types.h` | Shared type definitions, enumerations, constants |
| `SOMEIP_Serializer` | `someip_serializer.h/.c` | Wire encoding / decoding, header validation |
| `SOMEIP_SD` | `someip_sd.h/.c` | Service Discovery frame build and receive dispatch |
| `SOMEIP_Transport` | *(integration stub)* | UDP send – platform-provided via `SomeIpSd_Transmit()` |

### 3.2 Module Dependency Graph

```
┌──────────────────────────────┐
│  Application Software (ASW)  │
└──────────────┬───────────────┘
               │ calls
   ┌───────────▼───────────┐
   │     SOMEIP_SD         │
   │  someip_sd.h/.c       │──────► SomeIpSd_Transmit()
   └───────────┬───────────┘        (platform integration)
               │ calls
   ┌───────────▼───────────┐
   │  SOMEIP_Serializer    │
   │  someip_serializer.c  │
   └───────────┬───────────┘
               │ uses types from
   ┌───────────▼───────────┐
   │    SOMEIP_Types       │
   │    someip_types.h     │
   └───────────────────────┘
```

---

## 4. Interface Definitions

### 4.1 SOMEIP_Serializer Interfaces

#### SomeIp_Serialize

```c
Std_ReturnType SomeIp_Serialize(
    const SomeIp_Message_t *msg,      /* [in]  message to encode        */
    uint8_t                *buf,      /* [out] output frame buffer      */
    uint32_t                buf_size, /* [in]  size of output buffer    */
    uint32_t               *out_len   /* [out] bytes written on E_OK    */
);
```

**Pre-conditions:**  
- `msg`, `buf`, `out_len` are non-NULL.  
- `msg->payload` is non-NULL when `msg->payload_length > 0`.  

**Post-conditions:**  
- On E_OK: `buf[0..out_len-1]` contains a valid SOME/IP frame.  
- On E_NOT_OK: `buf` contents are undefined; `*out_len` is 0.  

#### SomeIp_Deserialize

```c
Std_ReturnType SomeIp_Deserialize(
    const uint8_t    *buf,     /* [in]  raw frame bytes (caller owns)  */
    uint32_t          buf_len, /* [in]  number of valid bytes in buf   */
    SomeIp_Message_t *msg      /* [out] parsed message structure       */
);
```

**Pre-conditions:** `buf` and `msg` are non-NULL, `buf_len ≥ 16`.  
**Post-conditions:**  
- On E_OK: `msg->header` is populated; `msg->payload` points into `buf`.  
- The caller **must not** free or overwrite `buf` while `msg->payload` is in use.

#### SomeIp_ValidateHeader

```c
Std_ReturnType SomeIp_ValidateHeader(const SomeIp_Header_t *header);
```

**Pre-conditions:** `header` is non-NULL.  
**Post-conditions:** Returns E_OK iff all header fields are within spec.

---

### 4.2 SOMEIP_SD Interfaces

#### SomeIpSd_OfferService

```c
Std_ReturnType SomeIpSd_OfferService(
    SomeIp_ServiceId_t   service_id,
    SomeIp_InstanceId_t  instance_id,
    uint8_t              major_ver,
    uint32_t             minor_ver,
    uint32_t             ttl_sec        /* 0 = Stop Offer */
);
```

#### SomeIpSd_FindService

```c
Std_ReturnType SomeIpSd_FindService(
    SomeIp_ServiceId_t   service_id,
    SomeIp_InstanceId_t  instance_id,  /* 0xFFFF = any   */
    uint8_t              major_ver,    /* 0xFF   = any   */
    uint32_t             minor_ver     /* 0xFFFFFFFF = any */
);
```

#### SomeIpSd_SubscribeEventgroup

```c
Std_ReturnType SomeIpSd_SubscribeEventgroup(
    SomeIp_ServiceId_t      service_id,
    SomeIp_InstanceId_t     instance_id,
    SomeIp_EventGroupId_t   eventgroup_id,
    uint8_t                 major_ver,
    uint32_t                ttl_sec        /* 0 = Unsubscribe */
);
```

#### SomeIpSd_RxIndication

```c
Std_ReturnType SomeIpSd_RxIndication(const uint8_t *buf, uint32_t length);
```

#### Platform Interface (integration layer must provide)

```c
Std_ReturnType SomeIpSd_Transmit(const uint8_t *buf, uint32_t len);
```

---

## 5. Data Flow

```
[ASW calls OfferService]
        │
        ▼
sd_send_service_entry()
        │  builds payload byte array on stack
        │  fills SomeIp_Message_t
        ▼
SomeIp_Serialize()
        │  writes 16-byte header + payload into stack frame[]
        ▼
SomeIpSd_Transmit()          ← platform BSW hands to SoAd → UDP
        │
        ▼
  [returns E_OK / E_NOT_OK to ASW]
```

---

## 6. State Model – SD Session ID Counter

Per PRS_SOMEIPSD_00056 the session ID shall be monotonically increasing
and non-zero. The module maintains two independent 16-bit counters (one
per SD entry type group) as static variables, initialized to 1 and
wrapping back to 1 (skipping 0).

```
  Initial: session_id = 1
  On each send: session_id = (session_id == 0xFFFF) ? 1 : session_id + 1
```

---

## 7. Error Handling Strategy

| Condition | Detection | Response |
|-----------|-----------|----------|
| NULL pointer argument | Checked at entry of every public function | Return E_NOT_OK immediately |
| Buffer too small | Length check before memcpy | Return E_NOT_OK |
| Malformed incoming frame | Header validation in Deserialize | Return E_NOT_OK |
| Transport failure | Propagated return value from Transmit | Return E_NOT_OK to caller |
| Invalid Message Type | Checked in ValidateHeader | Return E_NOT_OK |

---

## 8. ASPICE Level 2 – Process Attribute Evidence

| PA | Evidence Work Product |
|----|-----------------------|
| PA 1.1 – Process performance | Source code, unit test results |
| PA 2.1 – Planning | This document + SOMEIP-SWE1-001 |
| PA 2.2 – Work product management | Git repository, version numbers in header |

### ASPICE Level 3 (aspirational) – Process Definition Evidence

| PA | Evidence Work Product |
|----|-----------------------|
| PA 3.1 – Process definition | SOMEIP-SWE1-001, SOMEIP-SWE2-001, test specs |
| PA 3.2 – Process deployment | CI/CD pipeline (GTest, static analysis) |

---

## 9. SWE.2 → SWE.1 Traceability

| SWE.2 Element | SWE.1 Requirements |
|---------------|--------------------|
| `SomeIp_Header_t` struct | SR-SOMEIP-001, SR-SOMEIP-002 |
| `SomeIp_Serialize()` | SR-SOMEIP-010, SR-SOMEIP-011, SR-SOMEIP-105 |
| `SomeIp_Deserialize()` | SR-SOMEIP-012, SR-SOMEIP-013, SR-SOMEIP-014 |
| `SomeIp_ValidateHeader()` | SR-SOMEIP-003, SR-SOMEIP-015, SR-SOMEIP-016 |
| `SomeIpSd_OfferService()` | SR-SOMEIP-020, SR-SOMEIP-021 |
| `SomeIpSd_FindService()` | SR-SOMEIP-022 |
| `SomeIpSd_SubscribeEventgroup()` | SR-SOMEIP-023, SR-SOMEIP-024 |
| `SomeIpSd_RxIndication()` | SR-SOMEIP-025, SR-SOMEIP-026 |
| `SomeIpSd_Transmit()` interface | SR-SOMEIP-030 |
| All modules – no heap | SR-SOMEIP-100 |
| All public APIs – NULL guard | SR-SOMEIP-104 |
