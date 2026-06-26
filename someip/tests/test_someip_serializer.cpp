/**
 * SWE.5 Unit Verification – SOMEIP_Serializer module
 *
 * Test IDs map to SOMEIP-SWE5-001 (SWE5-SER-xxx, SWE5-DES-xxx, SWE5-VAL-xxx,
 * SWE5-NF-xxx) documented in swe5_unit_verification.md.
 *
 * Coverage target: 100% statement, 100% branch (MC/DC for validators).
 */

#include <gtest/gtest.h>
#include <cstring>

extern "C" {
#include "someip_serializer.h"
}

/* =========================================================================
 * Helpers
 * ========================================================================= */

static SomeIp_Message_t make_request(uint16_t svc, uint16_t meth,
                                     uint16_t cli, uint16_t sess,
                                     const uint8_t *payload, uint32_t plen)
{
    SomeIp_Message_t msg{};
    msg.header.service_id        = svc;
    msg.header.method_id         = meth;
    msg.header.client_id         = cli;
    msg.header.session_id        = sess;
    msg.header.protocol_version  = SOMEIP_PROTOCOL_VERSION;
    msg.header.interface_version = SOMEIP_INTERFACE_VERSION;
    msg.header.message_type      = static_cast<uint8_t>(SOMEIP_MSG_REQUEST);
    msg.header.return_code       = static_cast<uint8_t>(SOMEIP_RC_OK);
    msg.payload                  = payload;
    msg.payload_length           = plen;
    return msg;
}

/* =========================================================================
 * SWE5-SER – Serialization tests
 * ========================================================================= */

class SerializerTest : public ::testing::Test {
protected:
    uint8_t  buf[256]{};
    uint32_t out_len{0};

    void SetUp() override {
        std::memset(buf, 0xAA, sizeof(buf));
        out_len = 0;
    }
};

/* SWE5-SER-001 – Header-only message (no payload) produces 16-byte frame */
TEST_F(SerializerTest, SWE5_SER_001_HeaderOnlyMessage)
{
    SomeIp_Message_t msg = make_request(0x1234, 0x0001, 0x0010, 0x0001, nullptr, 0);
    ASSERT_EQ(E_OK, SomeIp_Serialize(&msg, buf, sizeof(buf), &out_len));
    EXPECT_EQ(16u, out_len);
}

/* SWE5-SER-002 – Big-endian encoding of all header fields */
TEST_F(SerializerTest, SWE5_SER_002_BigEndianHeaderEncoding)
{
    SomeIp_Message_t msg = make_request(0x1234, 0x5678, 0xABCD, 0x0003, nullptr, 0);
    ASSERT_EQ(E_OK, SomeIp_Serialize(&msg, buf, sizeof(buf), &out_len));

    /* Service ID at offset 0 */
    EXPECT_EQ(0x12u, buf[0]);
    EXPECT_EQ(0x34u, buf[1]);
    /* Method ID at offset 2 */
    EXPECT_EQ(0x56u, buf[2]);
    EXPECT_EQ(0x78u, buf[3]);
    /* Length = 8 (no payload) at offset 4 */
    EXPECT_EQ(0x00u, buf[4]);
    EXPECT_EQ(0x00u, buf[5]);
    EXPECT_EQ(0x00u, buf[6]);
    EXPECT_EQ(0x08u, buf[7]);
    /* Client ID at offset 8 */
    EXPECT_EQ(0xABu, buf[8]);
    EXPECT_EQ(0xCDu, buf[9]);
    /* Session ID at offset 10 */
    EXPECT_EQ(0x00u, buf[10]);
    EXPECT_EQ(0x03u, buf[11]);
    /* Protocol Version */
    EXPECT_EQ(0x01u, buf[12]);
    /* Interface Version */
    EXPECT_EQ(0x01u, buf[13]);
    /* Message Type: REQUEST */
    EXPECT_EQ(0x00u, buf[14]);
    /* Return Code: OK */
    EXPECT_EQ(0x00u, buf[15]);
}

/* SWE5-SER-003 – Payload is appended correctly after header */
TEST_F(SerializerTest, SWE5_SER_003_PayloadAppended)
{
    const uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
    SomeIp_Message_t msg = make_request(0x0001, 0x0001, 0x0001, 0x0001,
                                         payload, sizeof(payload));
    ASSERT_EQ(E_OK, SomeIp_Serialize(&msg, buf, sizeof(buf), &out_len));
    EXPECT_EQ(20u, out_len);  /* 16 header + 4 payload */
    /* Length field = 8 + 4 = 12 */
    EXPECT_EQ(0x00u, buf[4]);
    EXPECT_EQ(0x00u, buf[5]);
    EXPECT_EQ(0x00u, buf[6]);
    EXPECT_EQ(0x0Cu, buf[7]);
    /* Payload content */
    EXPECT_EQ(0xDEu, buf[16]);
    EXPECT_EQ(0xADu, buf[17]);
    EXPECT_EQ(0xBEu, buf[18]);
    EXPECT_EQ(0xEFu, buf[19]);
}

/* SWE5-SER-004 – Buffer too small returns E_NOT_OK (SR-SOMEIP-011) */
TEST_F(SerializerTest, SWE5_SER_004_BufferTooSmall)
{
    const uint8_t payload[10]{};
    SomeIp_Message_t msg = make_request(0x0001, 0x0001, 0x0001, 0x0001,
                                         payload, sizeof(payload));
    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(&msg, buf, 10, &out_len));
}

/* SWE5-SER-005 – Exact-fit buffer succeeds */
TEST_F(SerializerTest, SWE5_SER_005_ExactFitBuffer)
{
    SomeIp_Message_t msg = make_request(0x0001, 0x0001, 0x0001, 0x0001, nullptr, 0);
    EXPECT_EQ(E_OK, SomeIp_Serialize(&msg, buf, 16, &out_len));
    EXPECT_EQ(16u, out_len);
}

/* SWE5-SER-006 – NULL msg returns E_NOT_OK (SR-SOMEIP-104) */
TEST_F(SerializerTest, SWE5_SER_006_NullMsg)
{
    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(nullptr, buf, sizeof(buf), &out_len));
}

/* SWE5-SER-007 – NULL buf returns E_NOT_OK (SR-SOMEIP-104) */
TEST_F(SerializerTest, SWE5_SER_007_NullBuf)
{
    SomeIp_Message_t msg = make_request(0x0001, 0x0001, 0x0001, 0x0001, nullptr, 0);
    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(&msg, nullptr, sizeof(buf), &out_len));
}

/* SWE5-SER-008 – NULL out_len returns E_NOT_OK (SR-SOMEIP-104) */
TEST_F(SerializerTest, SWE5_SER_008_NullOutLen)
{
    SomeIp_Message_t msg = make_request(0x0001, 0x0001, 0x0001, 0x0001, nullptr, 0);
    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(&msg, buf, sizeof(buf), nullptr));
}

/* =========================================================================
 * SWE5-DES – Deserialization tests
 * ========================================================================= */

class DeserializerTest : public ::testing::Test {
protected:
    /* Canonical 20-byte REQUEST frame with 4-byte payload 0xDEADBEEF */
    static constexpr uint8_t kFrame[20] = {
        0x12, 0x34,              /* Service ID      */
        0x00, 0x01,              /* Method ID       */
        0x00, 0x00, 0x00, 0x0C, /* Length = 12     */
        0x00, 0x10,              /* Client ID       */
        0x00, 0x02,              /* Session ID      */
        0x01,                    /* Protocol Ver    */
        0x01,                    /* Interface Ver   */
        0x00,                    /* Message Type REQUEST */
        0x00,                    /* Return Code OK  */
        0xDE, 0xAD, 0xBE, 0xEF  /* Payload         */
    };

    SomeIp_Message_t msg{};
};

constexpr uint8_t DeserializerTest::kFrame[20];

/* SWE5-DES-001 – Valid frame deserializes all header fields correctly */
TEST_F(DeserializerTest, SWE5_DES_001_ValidFrameParsed)
{
    ASSERT_EQ(E_OK, SomeIp_Deserialize(kFrame, sizeof(kFrame), &msg));
    EXPECT_EQ(0x1234u, msg.header.service_id);
    EXPECT_EQ(0x0001u, msg.header.method_id);
    EXPECT_EQ(0x0010u, msg.header.client_id);
    EXPECT_EQ(0x0002u, msg.header.session_id);
    EXPECT_EQ(0x01u,   msg.header.protocol_version);
    EXPECT_EQ(0x01u,   msg.header.interface_version);
    EXPECT_EQ(static_cast<uint8_t>(SOMEIP_MSG_REQUEST), msg.header.message_type);
    EXPECT_EQ(static_cast<uint8_t>(SOMEIP_RC_OK),       msg.header.return_code);
}

/* SWE5-DES-002 – Payload pointer references input buffer (zero-copy, SR-SOMEIP-013) */
TEST_F(DeserializerTest, SWE5_DES_002_ZeroCopyPayload)
{
    ASSERT_EQ(E_OK, SomeIp_Deserialize(kFrame, sizeof(kFrame), &msg));
    ASSERT_NE(nullptr, msg.payload);
    EXPECT_EQ(4u, msg.payload_length);
    EXPECT_EQ(&kFrame[16], msg.payload);  /* points into the input buffer */
    EXPECT_EQ(0xDEu, msg.payload[0]);
    EXPECT_EQ(0xADu, msg.payload[1]);
    EXPECT_EQ(0xBEu, msg.payload[2]);
    EXPECT_EQ(0xEFu, msg.payload[3]);
}

/* SWE5-DES-003 – Frame shorter than 16 bytes returns E_NOT_OK (SR-SOMEIP-014) */
TEST_F(DeserializerTest, SWE5_DES_003_FrameTooShort)
{
    EXPECT_EQ(E_NOT_OK, SomeIp_Deserialize(kFrame, 15, &msg));
}

/* SWE5-DES-004 – Advertised length exceeds actual buffer returns E_NOT_OK */
TEST_F(DeserializerTest, SWE5_DES_004_LengthExceedsBuffer)
{
    /* Claim length = 100 but only provide 20 bytes */
    uint8_t bad[20];
    std::memcpy(bad, kFrame, 20);
    bad[4] = 0x00; bad[5] = 0x00; bad[6] = 0x00; bad[7] = 0x64; /* length = 100 */
    EXPECT_EQ(E_NOT_OK, SomeIp_Deserialize(bad, 20, &msg));
}

/* SWE5-DES-005 – NULL buf returns E_NOT_OK (SR-SOMEIP-104) */
TEST_F(DeserializerTest, SWE5_DES_005_NullBuf)
{
    EXPECT_EQ(E_NOT_OK, SomeIp_Deserialize(nullptr, 20, &msg));
}

/* SWE5-DES-006 – NULL msg returns E_NOT_OK (SR-SOMEIP-104) */
TEST_F(DeserializerTest, SWE5_DES_006_NullMsg)
{
    EXPECT_EQ(E_NOT_OK, SomeIp_Deserialize(kFrame, sizeof(kFrame), nullptr));
}

/* SWE5-DES-007 – Header-only frame (length = 8, no payload) */
TEST_F(DeserializerTest, SWE5_DES_007_NoPayload)
{
    uint8_t hdr_only[16];
    std::memcpy(hdr_only, kFrame, 16);
    hdr_only[4] = 0x00; hdr_only[5] = 0x00; hdr_only[6] = 0x00; hdr_only[7] = 0x08;
    ASSERT_EQ(E_OK, SomeIp_Deserialize(hdr_only, 16, &msg));
    EXPECT_EQ(0u,      msg.payload_length);
    EXPECT_EQ(nullptr, msg.payload);
}

/* SWE5-DES-008 – Round-trip: serialize then deserialize preserves all fields */
TEST_F(DeserializerTest, SWE5_DES_008_RoundTrip)
{
    const uint8_t pay[] = {0x01, 0x02, 0x03};
    SomeIp_Message_t original{};
    original.header.service_id        = 0xABCDu;
    original.header.method_id         = 0x1234u;
    original.header.client_id         = 0x0042u;
    original.header.session_id        = 0x0007u;
    original.header.protocol_version  = SOMEIP_PROTOCOL_VERSION;
    original.header.interface_version = 0x02u;
    original.header.message_type      = static_cast<uint8_t>(SOMEIP_MSG_RESPONSE);
    original.header.return_code       = static_cast<uint8_t>(SOMEIP_RC_OK);
    original.payload                  = pay;
    original.payload_length           = sizeof(pay);

    uint8_t  frame[64]{};
    uint32_t flen = 0;
    ASSERT_EQ(E_OK, SomeIp_Serialize(&original, frame, sizeof(frame), &flen));

    SomeIp_Message_t parsed{};
    ASSERT_EQ(E_OK, SomeIp_Deserialize(frame, flen, &parsed));

    EXPECT_EQ(original.header.service_id,        parsed.header.service_id);
    EXPECT_EQ(original.header.method_id,         parsed.header.method_id);
    EXPECT_EQ(original.header.client_id,         parsed.header.client_id);
    EXPECT_EQ(original.header.session_id,        parsed.header.session_id);
    EXPECT_EQ(original.header.protocol_version,  parsed.header.protocol_version);
    EXPECT_EQ(original.header.interface_version, parsed.header.interface_version);
    EXPECT_EQ(original.header.message_type,      parsed.header.message_type);
    EXPECT_EQ(original.header.return_code,       parsed.header.return_code);
    ASSERT_EQ(original.payload_length,           parsed.payload_length);
    EXPECT_EQ(0, std::memcmp(pay, parsed.payload, sizeof(pay)));
}

/* =========================================================================
 * SWE5-VAL – Header validation tests
 * ========================================================================= */

class ValidatorTest : public ::testing::Test {
protected:
    SomeIp_Header_t make_valid_header()
    {
        SomeIp_Header_t h{};
        h.service_id        = 0x0001;
        h.method_id         = 0x0001;
        h.length            = 8;
        h.client_id         = 0x0001;
        h.session_id        = 0x0001;
        h.protocol_version  = SOMEIP_PROTOCOL_VERSION;
        h.interface_version = SOMEIP_INTERFACE_VERSION;
        h.message_type      = static_cast<uint8_t>(SOMEIP_MSG_REQUEST);
        h.return_code       = static_cast<uint8_t>(SOMEIP_RC_OK);
        return h;
    }
};

/* SWE5-VAL-001 – Valid header returns E_OK */
TEST_F(ValidatorTest, SWE5_VAL_001_ValidHeader)
{
    SomeIp_Header_t h = make_valid_header();
    EXPECT_EQ(E_OK, SomeIp_ValidateHeader(&h));
}

/* SWE5-VAL-002 – Wrong protocol version returns E_NOT_OK (SR-SOMEIP-003) */
TEST_F(ValidatorTest, SWE5_VAL_002_WrongProtocolVersion)
{
    SomeIp_Header_t h = make_valid_header();
    h.protocol_version = 0x02u;
    EXPECT_EQ(E_NOT_OK, SomeIp_ValidateHeader(&h));
}

/* SWE5-VAL-003 – Length < 8 returns E_NOT_OK (SR-SOMEIP-015) */
TEST_F(ValidatorTest, SWE5_VAL_003_LengthTooSmall)
{
    SomeIp_Header_t h = make_valid_header();
    h.length = 7u;
    EXPECT_EQ(E_NOT_OK, SomeIp_ValidateHeader(&h));
}

/* SWE5-VAL-004 – Each valid message type returns E_OK (SR-SOMEIP-016) */
TEST_F(ValidatorTest, SWE5_VAL_004_AllValidMessageTypes)
{
    const uint8_t valid_types[] = {
        0x00, 0x01, 0x02, 0x80, 0x81,
        0x20, 0x21, 0x22, 0xA0, 0xA1
    };
    for (auto t : valid_types) {
        SomeIp_Header_t h = make_valid_header();
        h.message_type = t;
        EXPECT_EQ(E_OK, SomeIp_ValidateHeader(&h)) << "Failed for type 0x" << std::hex << (int)t;
    }
}

/* SWE5-VAL-005 – Undefined message type returns E_NOT_OK (SR-SOMEIP-016) */
TEST_F(ValidatorTest, SWE5_VAL_005_InvalidMessageType)
{
    const uint8_t invalid_types[] = {0x03, 0x04, 0x05, 0x10, 0xFF};
    for (auto t : invalid_types) {
        SomeIp_Header_t h = make_valid_header();
        h.message_type = t;
        EXPECT_EQ(E_NOT_OK, SomeIp_ValidateHeader(&h)) << "Expected FAIL for type 0x" << std::hex << (int)t;
    }
}

/* SWE5-VAL-006 – NULL header returns E_NOT_OK (SR-SOMEIP-104) */
TEST_F(ValidatorTest, SWE5_VAL_006_NullHeader)
{
    EXPECT_EQ(E_NOT_OK, SomeIp_ValidateHeader(nullptr));
}

/* SWE5-VAL-007 – Length == 8 (boundary) is valid */
TEST_F(ValidatorTest, SWE5_VAL_007_LengthBoundaryExactlyEight)
{
    SomeIp_Header_t h = make_valid_header();
    h.length = 8u;
    EXPECT_EQ(E_OK, SomeIp_ValidateHeader(&h));
}

/* =========================================================================
 * SWE5-NF – Non-functional requirement tests
 * ========================================================================= */

/* SWE5-NF-001 – All public functions handle NULL gracefully (SR-SOMEIP-104) */
TEST(NonFunctionalTest, SWE5_NF_001_NullSafetyAllAPIs)
{
    uint32_t len = 0;
    uint8_t  buf[32]{};
    SomeIp_Message_t msg{};

    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(nullptr, buf, 32, &len));
    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(&msg,  nullptr, 32, &len));
    EXPECT_EQ(E_NOT_OK, SomeIp_Serialize(&msg,  buf, 32, nullptr));
    EXPECT_EQ(E_NOT_OK, SomeIp_Deserialize(nullptr, 32, &msg));
    EXPECT_EQ(E_NOT_OK, SomeIp_Deserialize(buf, 32, nullptr));
    EXPECT_EQ(E_NOT_OK, SomeIp_ValidateHeader(nullptr));
}

/* SWE5-NF-002 – Serialized frame is always 16 + payload_length bytes */
TEST(NonFunctionalTest, SWE5_NF_002_FrameSizeIsHeaderPlusPayload)
{
    uint8_t  buf[512]{};
    uint32_t out_len = 0;
    const uint8_t payload[100]{};

    SomeIp_Message_t msg{};
    msg.header.protocol_version  = SOMEIP_PROTOCOL_VERSION;
    msg.header.interface_version = SOMEIP_INTERFACE_VERSION;
    msg.header.message_type      = static_cast<uint8_t>(SOMEIP_MSG_REQUEST);
    msg.header.length            = 8;

    for (uint32_t plen : {0u, 1u, 4u, 16u, 100u}) {
        msg.payload        = (plen > 0) ? payload : nullptr;
        msg.payload_length = plen;
        ASSERT_EQ(E_OK, SomeIp_Serialize(&msg, buf, sizeof(buf), &out_len));
        EXPECT_EQ(16u + plen, out_len) << "Failed for payload_length=" << plen;
    }
}
