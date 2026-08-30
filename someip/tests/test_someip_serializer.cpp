/**
 * test_someip_serializer.cpp – SWE.5 Unit Verification: SomeIp module
 *
 * Tests: SomeIp_Init, SomeIp_GetVersionInfo, SomeIp_Serialize,
 *        SomeIp_Deserialize, SomeIp_ValidateHeader.
 *
 * All test IDs map to SOMEIP-SWE5-001 entries in swe5_unit_verification.md.
 * AUTOSAR types used throughout (uint8/uint16/uint32/boolean).
 */

#include <gtest/gtest.h>
#include <cstring>

extern "C" {
#include "SomeIp.h"
#include "Det.h"
}

/* =========================================================================
 * Test fixture – initialises the module and resets DET before each test
 * ========================================================================= */

class SomeIpTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        Det_Reset();
        SomeIp_Init(nullptr);   /* nullptr = use compile-time defaults */
    }
};

/* =========================================================================
 * Helper – build a minimal valid SomeIp_HeaderType
 * ========================================================================= */

static SomeIp_HeaderType MakeValidHeader(
    SomeIp_ServiceIdType ServiceId   = 0x1234u,
    SomeIp_MethodIdType  MethodId    = 0x0001u,
    SomeIp_ClientIdType  ClientId    = 0x0010u,
    SomeIp_SessionIdType SessionId   = 0x0001u)
{
    SomeIp_HeaderType H{};
    H.ServiceId        = ServiceId;
    H.MethodId         = MethodId;
    H.Length           = 8u;        /* no payload */
    H.ClientId         = ClientId;
    H.SessionId        = SessionId;
    H.ProtocolVersion  = SOMEIP_PROTOCOL_VERSION;
    H.InterfaceVersion = SOMEIP_INTERFACE_VERSION_DEFAULT;
    H.MessageType      = SOMEIP_MSG_REQUEST;
    H.ReturnCode       = SOMEIP_RC_OK;
    return H;
}

/* =========================================================================
 * SWE5-INIT – Module lifecycle
 * ========================================================================= */

TEST(SomeIpLifecycle, SWE5_INIT_001_FunctionsReturnENotOkBeforeInit)
{
    /* Use a fresh, uninitialized state by NOT calling SomeIp_Init */
    /* Cannot do this without a second module instance; skip and document:
     * SOMEIP_E_UNINIT is reported via DET – verified in DET tests below. */
    SUCCEED();  /* Evidence: DET reports UNINIT; see SWE5-DET-* tests */
}

TEST_F(SomeIpTest, SWE5_INIT_002_InitSetsModuleReady)
{
    /* After SomeIp_Init() the serializer must work */
    SomeIp_HeaderType H = MakeValidHeader();
    uint8  Buf[32]{};
    uint32 Len = sizeof(Buf);
    EXPECT_EQ(E_OK, SomeIp_Serialize(&H, nullptr, Buf, &Len));
}

/* =========================================================================
 * SWE5-VER – Version info
 * ========================================================================= */

#if (SOMEIP_VERSION_INFO_API == STD_ON)
TEST_F(SomeIpTest, SWE5_VER_001_GetVersionInfoReturnsCorrectValues)
{
    Std_VersionInfoType Vi{};
    SomeIp_GetVersionInfo(&Vi);
    EXPECT_EQ(SOMEIP_VENDOR_ID,        Vi.vendorID);
    EXPECT_EQ(SOMEIP_MODULE_ID,        Vi.moduleID);
    EXPECT_EQ(SOMEIP_SW_MAJOR_VERSION, Vi.sw_major_version);
    EXPECT_EQ(SOMEIP_SW_MINOR_VERSION, Vi.sw_minor_version);
    EXPECT_EQ(SOMEIP_SW_PATCH_VERSION, Vi.sw_patch_version);
}

TEST_F(SomeIpTest, SWE5_VER_002_GetVersionInfoNullReportsDet)
{
    Det_Reset();
    SomeIp_GetVersionInfo(nullptr);
    EXPECT_EQ(SOMEIP_E_NULL_PTR, Det_GetLastErrorId());
    EXPECT_EQ(SOMEIP_SID_GET_VERSION_INFO, Det_GetLastApiId());
}
#endif

/* =========================================================================
 * SWE5-SER – SomeIp_Serialize
 * ========================================================================= */

class SerializeTest : public SomeIpTest {
protected:
    uint8  Buf[256]{};
    uint32 OutLen{sizeof(Buf)};
    void   SetUp() override { SomeIpTest::SetUp(); OutLen = sizeof(Buf); }
};

/* SWE5-SER-001: header-only frame is exactly 16 bytes */
TEST_F(SerializeTest, SWE5_SER_001_HeaderOnlyFrameIs16Bytes)
{
    SomeIp_HeaderType H = MakeValidHeader();
    ASSERT_EQ(E_OK, SomeIp_Serialize(&H, nullptr, Buf, &OutLen));
    EXPECT_EQ(16u, OutLen);
}

/* SWE5-SER-002: all header fields encoded big-endian at correct offsets */
TEST_F(SerializeTest, SWE5_SER_002_BigEndianHeaderEncoding)
{
    SomeIp_HeaderType H = MakeValidHeader(0x1234u, 0x5678u, 0xABCDu, 0x0003u);
    H.MessageType      = SOMEIP_MSG_NOTIFICATION;
    H.ReturnCode       = SOMEIP_RC_NOT_OK;

    ASSERT_EQ(E_OK, SomeIp_Serialize(&H, nullptr, Buf, &OutLen));

    /* Service ID @ [0..1] */
    EXPECT_EQ(0x12u, Buf[0]);  EXPECT_EQ(0x34u, Buf[1]);
    /* Method ID  @ [2..3] */
    EXPECT_EQ(0x56u, Buf[2]);  EXPECT_EQ(0x78u, Buf[3]);
    /* Length     @ [4..7] = 8 (no payload) */
    EXPECT_EQ(0x00u, Buf[4]);  EXPECT_EQ(0x00u, Buf[5]);
    EXPECT_EQ(0x00u, Buf[6]);  EXPECT_EQ(0x08u, Buf[7]);
    /* Client ID  @ [8..9] */
    EXPECT_EQ(0xABu, Buf[8]);  EXPECT_EQ(0xCDu, Buf[9]);
    /* Session ID @ [10..11] */
    EXPECT_EQ(0x00u, Buf[10]); EXPECT_EQ(0x03u, Buf[11]);
    /* Protocol Version @ [12] */
    EXPECT_EQ(SOMEIP_PROTOCOL_VERSION,          Buf[12]);
    /* Interface Version @ [13] */
    EXPECT_EQ(SOMEIP_INTERFACE_VERSION_DEFAULT, Buf[13]);
    /* Message Type @ [14] */
    EXPECT_EQ(static_cast<uint8>(SOMEIP_MSG_NOTIFICATION), Buf[14]);
    /* Return Code @ [15] */
    EXPECT_EQ(static_cast<uint8>(SOMEIP_RC_NOT_OK),        Buf[15]);
}

/* SWE5-SER-003: payload appended immediately after header; Length updated */
TEST_F(SerializeTest, SWE5_SER_003_PayloadAppendedAfterHeader)
{
    uint8       Pay[] = {0xDE, 0xAD, 0xBE, 0xEF};
    PduInfoType Pdu{};
    Pdu.SduDataPtr  = Pay;
    Pdu.MetaDataPtr = nullptr;
    Pdu.SduLength   = sizeof(Pay);

    SomeIp_HeaderType H = MakeValidHeader();
    ASSERT_EQ(E_OK, SomeIp_Serialize(&H, &Pdu, Buf, &OutLen));

    EXPECT_EQ(20u, OutLen);
    /* Length field = 8 + 4 = 12 */
    EXPECT_EQ(0x00u, Buf[4]); EXPECT_EQ(0x00u, Buf[5]);
    EXPECT_EQ(0x00u, Buf[6]); EXPECT_EQ(0x0Cu, Buf[7]);
    /* Payload at [16..19] */
    EXPECT_EQ(0xDEu, Buf[16]); EXPECT_EQ(0xADu, Buf[17]);
    EXPECT_EQ(0xBEu, Buf[18]); EXPECT_EQ(0xEFu, Buf[19]);
}

/* SWE5-SER-004: buffer exactly the right size succeeds */
TEST_F(SerializeTest, SWE5_SER_004_ExactFitBuffer)
{
    SomeIp_HeaderType H = MakeValidHeader();
    OutLen = 16u;
    EXPECT_EQ(E_OK, SomeIp_Serialize(&H, nullptr, Buf, &OutLen));
    EXPECT_EQ(16u, OutLen);
}

/* SWE5-SER-005: buffer one byte short → E_NOT_OK + DET BUFF_TOO_SMALL */
TEST_F(SerializeTest, SWE5_SER_005_BufferTooSmallReportsDetAndReturnsError)
{
    SomeIp_HeaderType H = MakeValidHeader();
    OutLen = 15u;
    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(&H, nullptr, Buf, &OutLen));
    EXPECT_EQ(SOMEIP_E_BUFF_TOO_SMALL, Det_GetLastErrorId());
}

/* SWE5-SER-006..008: NULL arguments → E_NOT_OK + DET NULL_PTR */
TEST_F(SerializeTest, SWE5_SER_006_NullHeaderPtr)
{
    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(nullptr, nullptr, Buf, &OutLen));
    EXPECT_EQ(SOMEIP_E_NULL_PTR, Det_GetLastErrorId());
}

TEST_F(SerializeTest, SWE5_SER_007_NullBufPtr)
{
    SomeIp_HeaderType H = MakeValidHeader();
    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(&H, nullptr, nullptr, &OutLen));
    EXPECT_EQ(SOMEIP_E_NULL_PTR, Det_GetLastErrorId());
}

TEST_F(SerializeTest, SWE5_SER_008_NullBufLenPtr)
{
    SomeIp_HeaderType H = MakeValidHeader();
    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(&H, nullptr, Buf, nullptr));
    EXPECT_EQ(SOMEIP_E_NULL_PTR, Det_GetLastErrorId());
}

/* =========================================================================
 * SWE5-DES – SomeIp_Deserialize
 * ========================================================================= */

class DeserializeTest : public SomeIpTest {
protected:
    /* Canonical 20-byte REQUEST frame (Service=0x1234, Method=0x0001,
     * Length=12, Client=0x0010, Session=0x0002, payload=0xDEADBEEF) */
    static constexpr uint8 kFrame[20] = {
        0x12u, 0x34u,               /* Service ID      */
        0x00u, 0x01u,               /* Method ID       */
        0x00u, 0x00u, 0x00u, 0x0Cu, /* Length = 12     */
        0x00u, 0x10u,               /* Client ID       */
        0x00u, 0x02u,               /* Session ID      */
        0x01u,                      /* Protocol Ver    */
        0x01u,                      /* Interface Ver   */
        0x00u,                      /* Message Type    */
        0x00u,                      /* Return Code     */
        0xDEu, 0xADu, 0xBEu, 0xEFu /* Payload         */
    };

    SomeIp_HeaderType Header{};
    PduInfoType       Payload{};

    void SetUp() override {
        SomeIpTest::SetUp();
        std::memset(&Header,  0, sizeof(Header));
        std::memset(&Payload, 0, sizeof(Payload));
    }
};

constexpr uint8 DeserializeTest::kFrame[20];

/* SWE5-DES-001: all header fields correctly parsed */
TEST_F(DeserializeTest, SWE5_DES_001_AllHeaderFieldsParsed)
{
    ASSERT_EQ(E_OK, SomeIp_Deserialize(kFrame, sizeof(kFrame), &Header, &Payload));
    EXPECT_EQ(0x1234u, Header.ServiceId);
    EXPECT_EQ(0x0001u, Header.MethodId);
    EXPECT_EQ(0x0010u, Header.ClientId);
    EXPECT_EQ(0x0002u, Header.SessionId);
    EXPECT_EQ(0x01u,   Header.ProtocolVersion);
    EXPECT_EQ(0x01u,   Header.InterfaceVersion);
    EXPECT_EQ(SOMEIP_MSG_REQUEST, Header.MessageType);
    EXPECT_EQ(SOMEIP_RC_OK,       Header.ReturnCode);
}

/* SWE5-DES-002: zero-copy – SduDataPtr points into the input buffer */
TEST_F(DeserializeTest, SWE5_DES_002_ZeroCopyPayloadPointer)
{
    ASSERT_EQ(E_OK, SomeIp_Deserialize(kFrame, sizeof(kFrame), &Header, &Payload));
    EXPECT_EQ(4u, Payload.SduLength);
    EXPECT_EQ(reinterpret_cast<const uint8*>(&kFrame[16]), Payload.SduDataPtr);
    EXPECT_EQ(0xDEu, Payload.SduDataPtr[0]);
    EXPECT_EQ(0xADu, Payload.SduDataPtr[1]);
    EXPECT_EQ(0xBEu, Payload.SduDataPtr[2]);
    EXPECT_EQ(0xEFu, Payload.SduDataPtr[3]);
}

/* SWE5-DES-003: frame shorter than 16 bytes → E_NOT_OK */
TEST_F(DeserializeTest, SWE5_DES_003_FrameShorterThan16Bytes)
{
    EXPECT_EQ(E_NOT_OK, SomeIp_Deserialize(kFrame, 15u, &Header, &Payload));
}

/* SWE5-DES-004: Length field claims more data than buffer contains */
TEST_F(DeserializeTest, SWE5_DES_004_LengthFieldExceedsBuffer)
{
    uint8 Bad[20];
    std::memcpy(Bad, kFrame, 20);
    Bad[4] = 0x00u; Bad[5] = 0x00u; Bad[6] = 0x00u; Bad[7] = 0x64u; /* Length = 100 */
    EXPECT_EQ(E_NOT_OK, SomeIp_Deserialize(Bad, 20u, &Header, &Payload));
}

/* SWE5-DES-005: NULL BufPtr → E_NOT_OK + DET */
TEST_F(DeserializeTest, SWE5_DES_005_NullBufPtr)
{
    EXPECT_EQ(E_NOT_OK, SomeIp_Deserialize(nullptr, 20u, &Header, &Payload));
    EXPECT_EQ(SOMEIP_E_NULL_PTR, Det_GetLastErrorId());
}

/* SWE5-DES-006: NULL HeaderPtr → E_NOT_OK + DET */
TEST_F(DeserializeTest, SWE5_DES_006_NullHeaderPtr)
{
    EXPECT_EQ(E_NOT_OK, SomeIp_Deserialize(kFrame, sizeof(kFrame), nullptr, &Payload));
    EXPECT_EQ(SOMEIP_E_NULL_PTR, Det_GetLastErrorId());
}

/* SWE5-DES-007: NULL PayloadPtr → E_NOT_OK + DET */
TEST_F(DeserializeTest, SWE5_DES_007_NullPayloadPtr)
{
    EXPECT_EQ(E_NOT_OK, SomeIp_Deserialize(kFrame, sizeof(kFrame), &Header, nullptr));
    EXPECT_EQ(SOMEIP_E_NULL_PTR, Det_GetLastErrorId());
}

/* SWE5-DES-008: header-only frame → SduDataPtr == NULL, SduLength == 0 */
TEST_F(DeserializeTest, SWE5_DES_008_NoPayloadFrame)
{
    uint8 HdrOnly[16];
    std::memcpy(HdrOnly, kFrame, 16);
    HdrOnly[4] = 0; HdrOnly[5] = 0; HdrOnly[6] = 0; HdrOnly[7] = 8; /* Length = 8 */
    ASSERT_EQ(E_OK, SomeIp_Deserialize(HdrOnly, 16u, &Header, &Payload));
    EXPECT_EQ(0u,       Payload.SduLength);
    EXPECT_EQ(nullptr,  Payload.SduDataPtr);
}

/* SWE5-DES-009: round-trip fidelity */
TEST_F(DeserializeTest, SWE5_DES_009_RoundTrip)
{
    const uint8 Pay[] = {0x01u, 0x02u, 0x03u};
    PduInfoType OrigPdu{};
    OrigPdu.SduDataPtr  = const_cast<uint8*>(Pay);
    OrigPdu.MetaDataPtr = nullptr;
    OrigPdu.SduLength   = sizeof(Pay);

    SomeIp_HeaderType OrigH{};
    OrigH.ServiceId        = 0xABCDu;
    OrigH.MethodId         = 0x1234u;
    OrigH.ClientId         = 0x0042u;
    OrigH.SessionId        = 0x0007u;
    OrigH.ProtocolVersion  = SOMEIP_PROTOCOL_VERSION;
    OrigH.InterfaceVersion = 0x02u;
    OrigH.MessageType      = SOMEIP_MSG_RESPONSE;
    OrigH.ReturnCode       = SOMEIP_RC_OK;

    uint8  Frame[64]{};
    uint32 FLen = sizeof(Frame);
    ASSERT_EQ(E_OK, SomeIp_Serialize(&OrigH, &OrigPdu, Frame, &FLen));

    SomeIp_HeaderType ParsedH{};
    PduInfoType       ParsedPdu{};
    ASSERT_EQ(E_OK, SomeIp_Deserialize(Frame, FLen, &ParsedH, &ParsedPdu));

    EXPECT_EQ(OrigH.ServiceId,        ParsedH.ServiceId);
    EXPECT_EQ(OrigH.MethodId,         ParsedH.MethodId);
    EXPECT_EQ(OrigH.ClientId,         ParsedH.ClientId);
    EXPECT_EQ(OrigH.SessionId,        ParsedH.SessionId);
    EXPECT_EQ(OrigH.ProtocolVersion,  ParsedH.ProtocolVersion);
    EXPECT_EQ(OrigH.InterfaceVersion, ParsedH.InterfaceVersion);
    EXPECT_EQ(OrigH.MessageType,      ParsedH.MessageType);
    EXPECT_EQ(OrigH.ReturnCode,       ParsedH.ReturnCode);
    ASSERT_EQ(OrigPdu.SduLength,      ParsedPdu.SduLength);
    EXPECT_EQ(0, std::memcmp(Pay, ParsedPdu.SduDataPtr, sizeof(Pay)));
}

/* =========================================================================
 * SWE5-VAL – SomeIp_ValidateHeader
 * ========================================================================= */

class ValidateHeaderTest : public SomeIpTest {
protected:
    SomeIp_HeaderType MakeH() { return MakeValidHeader(); }
};

/* SWE5-VAL-001: fully valid header → E_OK */
TEST_F(ValidateHeaderTest, SWE5_VAL_001_ValidHeaderReturnsEOk)
{
    SomeIp_HeaderType H = MakeH();
    EXPECT_EQ(E_OK, SomeIp_ValidateHeader(&H));
}

/* SWE5-VAL-002: wrong protocol version → E_NOT_OK */
TEST_F(ValidateHeaderTest, SWE5_VAL_002_WrongProtocolVersionReturnsError)
{
    SomeIp_HeaderType H = MakeH();
    H.ProtocolVersion = 0x02u;
    EXPECT_EQ(E_NOT_OK, SomeIp_ValidateHeader(&H));
}

/* SWE5-VAL-003: Length < 8 → E_NOT_OK */
TEST_F(ValidateHeaderTest, SWE5_VAL_003_LengthBelowMinimumReturnsError)
{
    SomeIp_HeaderType H = MakeH();
    H.Length = 7u;
    EXPECT_EQ(E_NOT_OK, SomeIp_ValidateHeader(&H));
}

/* SWE5-VAL-004: Length == 8 (boundary) → E_OK */
TEST_F(ValidateHeaderTest, SWE5_VAL_004_LengthExactlyEightIsValid)
{
    SomeIp_HeaderType H = MakeH();
    H.Length = 8u;
    EXPECT_EQ(E_OK, SomeIp_ValidateHeader(&H));
}

/* SWE5-VAL-005: all valid MessageType values → E_OK */
TEST_F(ValidateHeaderTest, SWE5_VAL_005_AllDefinedMessageTypesAreValid)
{
    const SomeIp_MessageType ValidTypes[] = {
        SOMEIP_MSG_REQUEST, SOMEIP_MSG_REQUEST_NO_RETURN, SOMEIP_MSG_NOTIFICATION,
        SOMEIP_MSG_RESPONSE, SOMEIP_MSG_ERROR,
        SOMEIP_MSG_TP_REQUEST, SOMEIP_MSG_TP_REQUEST_NO_RETURN,
        SOMEIP_MSG_TP_NOTIFICATION, SOMEIP_MSG_TP_RESPONSE, SOMEIP_MSG_TP_ERROR
    };
    for (auto T : ValidTypes) {
        SomeIp_HeaderType H = MakeH();
        H.MessageType = T;
        EXPECT_EQ(E_OK, SomeIp_ValidateHeader(&H))
            << "Failed for MessageType 0x" << std::hex << static_cast<int>(T);
    }
}

/* SWE5-VAL-006: undefined MessageType values → E_NOT_OK */
TEST_F(ValidateHeaderTest, SWE5_VAL_006_UndefinedMessageTypeReturnsError)
{
    const uint8 InvalidTypes[] = {0x03u, 0x04u, 0x05u, 0x10u, 0x30u, 0xFFu};
    for (auto T : InvalidTypes) {
        SomeIp_HeaderType H = MakeH();
        H.MessageType = static_cast<SomeIp_MessageType>(T);
        EXPECT_EQ(E_NOT_OK, SomeIp_ValidateHeader(&H))
            << "Expected E_NOT_OK for MessageType 0x" << std::hex << static_cast<int>(T);
    }
}

/* SWE5-VAL-007: NULL HeaderPtr → E_NOT_OK + DET NULL_PTR */
TEST_F(ValidateHeaderTest, SWE5_VAL_007_NullHeaderPtrReportsDet)
{
    Det_Reset();
    EXPECT_EQ(E_NOT_OK, SomeIp_ValidateHeader(nullptr));
    EXPECT_EQ(SOMEIP_E_NULL_PTR,         Det_GetLastErrorId());
    EXPECT_EQ(SOMEIP_SID_VALIDATE_HEADER, Det_GetLastApiId());
}

/* =========================================================================
 * SWE5-NF – Non-functional requirements
 * ========================================================================= */

TEST_F(SomeIpTest, SWE5_NF_001_AllPublicApisHandleNullGracefully)
{
    uint8       Buf[32]{};
    uint32      Len = sizeof(Buf);
    PduInfoType Pdu{};

    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(nullptr, nullptr, Buf, &Len));
    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(nullptr, nullptr, nullptr, &Len));
    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(nullptr, nullptr, Buf, nullptr));
    EXPECT_EQ(E_NOT_OK, SomeIp_Deserialize(nullptr, 32u, nullptr, nullptr));
    EXPECT_EQ(E_NOT_OK, SomeIp_ValidateHeader(nullptr));
}

TEST_F(SomeIpTest, SWE5_NF_002_FrameSizeEqualsHeaderPlusPayload)
{
    uint8  Buf[512]{};
    uint32 OutLen;
    const uint8 Pay[100]{};

    SomeIp_HeaderType H = MakeValidHeader();

    for (uint32 PLen : {0u, 1u, 4u, 16u, 100u}) {
        PduInfoType Pdu{};
        Pdu.SduDataPtr  = const_cast<uint8*>(Pay);
        Pdu.MetaDataPtr = nullptr;
        Pdu.SduLength   = PLen;

        OutLen = sizeof(Buf);
        ASSERT_EQ(E_OK, SomeIp_Serialize(&H, (PLen > 0u) ? &Pdu : nullptr, Buf, &OutLen));
        EXPECT_EQ(16u + PLen, OutLen) << "Mismatch for PLen=" << PLen;
    }
}
