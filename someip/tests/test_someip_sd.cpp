/**
 * SWE.5 Unit Verification – SOMEIP_SD module
 *
 * Test IDs map to SOMEIP-SWE5-001 (SWE5-SD-xxx) documented in
 * swe5_unit_verification.md.
 *
 * The stub_transport.c captures every frame passed to SomeIpSd_Transmit()
 * so assertions can inspect wire-level encoding.
 */

#include <gtest/gtest.h>
#include <cstring>

extern "C" {
#include "someip_sd.h"
#include "someip_serializer.h"
#include "stub_transport.h"
}

/* =========================================================================
 * Helpers
 * ========================================================================= */

/* Read 16-bit big-endian from frame at given offset */
static uint16_t u16(const uint8_t *f, size_t off)
{
    return static_cast<uint16_t>((f[off] << 8u) | f[off + 1]);
}

/* Read 32-bit big-endian from frame at given offset */
static uint32_t u32(const uint8_t *f, size_t off)
{
    return (static_cast<uint32_t>(f[off])     << 24u) |
           (static_cast<uint32_t>(f[off + 1]) << 16u) |
           (static_cast<uint32_t>(f[off + 2]) <<  8u) |
            static_cast<uint32_t>(f[off + 3]);
}

/* Offsets within a captured SD frame */
static constexpr size_t kHdrServiceId  =  0;
static constexpr size_t kHdrMethodId   =  2;
static constexpr size_t kHdrLength     =  4;
static constexpr size_t kHdrMsgType    = 14;
static constexpr size_t kHdrRetCode    = 15;
static constexpr size_t kSdFlags       = 16;
static constexpr size_t kSdEntriesLen  = 20;
static constexpr size_t kSdEntry0      = 24; /* first entry starts here */

/* Type-1 entry field offsets relative to kSdEntry0 */
static constexpr size_t kE1Type       =  0;
static constexpr size_t kE1ServiceId  =  4;
static constexpr size_t kE1InstanceId =  6;
static constexpr size_t kE1MajorVer   =  8;
static constexpr size_t kE1TTL_hi     =  9; /* 3-byte TTL */
static constexpr size_t kE1MinorVer   = 12;

/* Type-2 entry additional fields */
static constexpr size_t kE2Counter      = 13;
static constexpr size_t kE2EventgroupId = 14;

class SdTest : public ::testing::Test {
protected:
    void SetUp() override { StubTransport_Reset(); }
};

/* =========================================================================
 * SWE5-SD-001 – OfferService produces correct SD frame
 * ========================================================================= */
TEST_F(SdTest, SWE5_SD_001_OfferServiceFrameStructure)
{
    ASSERT_EQ(E_OK, SomeIpSd_OfferService(0x1234, 0x0001, 0x01, 0x00000001, 3));

    ASSERT_EQ(1u, StubTransport_GetCallCount());
    const uint8_t *f  = StubTransport_GetLastFrame();
    uint32_t       fl = StubTransport_GetLastFrameLen();

    /* SOME/IP SD header */
    EXPECT_EQ(0xFFFFu, u16(f, kHdrServiceId));
    EXPECT_EQ(0x8100u, u16(f, kHdrMethodId));
    EXPECT_EQ(static_cast<uint8_t>(SOMEIP_MSG_NOTIFICATION), f[kHdrMsgType]);
    EXPECT_EQ(static_cast<uint8_t>(SOMEIP_RC_OK),            f[kHdrRetCode]);

    /* SD payload present */
    EXPECT_GT(fl, 16u + 8u + 16u);  /* header + SD flags/reserved/len + 1 entry */

    /* Entries array length = 16 */
    EXPECT_EQ(16u, u32(f, kSdEntriesLen));

    /* Entry type = Offer (0x01) */
    EXPECT_EQ(0x01u, f[kSdEntry0 + kE1Type]);

    /* Service / Instance IDs */
    EXPECT_EQ(0x1234u, u16(f, kSdEntry0 + kE1ServiceId));
    EXPECT_EQ(0x0001u, u16(f, kSdEntry0 + kE1InstanceId));

    /* Major version */
    EXPECT_EQ(0x01u, f[kSdEntry0 + kE1MajorVer]);

    /* TTL = 3 (24-bit big-endian) */
    EXPECT_EQ(0x00u, f[kSdEntry0 + kE1TTL_hi]);
    EXPECT_EQ(0x00u, f[kSdEntry0 + kE1TTL_hi + 1]);
    EXPECT_EQ(0x03u, f[kSdEntry0 + kE1TTL_hi + 2]);

    /* Minor version */
    EXPECT_EQ(1u, u32(f, kSdEntry0 + kE1MinorVer));
}

/* SWE5-SD-002 – StopOffer: OfferService with TTL=0 sets TTL field to 0 */
TEST_F(SdTest, SWE5_SD_002_StopOfferTTLIsZero)
{
    ASSERT_EQ(E_OK, SomeIpSd_OfferService(0x0001, 0x0001, 0x01, 0, 0));
    const uint8_t *f = StubTransport_GetLastFrame();
    EXPECT_EQ(0x00u, f[kSdEntry0 + kE1TTL_hi]);
    EXPECT_EQ(0x00u, f[kSdEntry0 + kE1TTL_hi + 1]);
    EXPECT_EQ(0x00u, f[kSdEntry0 + kE1TTL_hi + 2]);
}

/* SWE5-SD-003 – FindService produces entry type 0x00 */
TEST_F(SdTest, SWE5_SD_003_FindServiceEntryType)
{
    ASSERT_EQ(E_OK, SomeIpSd_FindService(0xAAAA, 0xFFFF, 0xFF, 0xFFFFFFFF));
    const uint8_t *f = StubTransport_GetLastFrame();
    EXPECT_EQ(0x00u, f[kSdEntry0 + kE1Type]);
    EXPECT_EQ(0xAAAAu, u16(f, kSdEntry0 + kE1ServiceId));
    EXPECT_EQ(0xFFFFu, u16(f, kSdEntry0 + kE1InstanceId));
    EXPECT_EQ(0xFFu,   f[kSdEntry0 + kE1MajorVer]);
    EXPECT_EQ(0xFFFFFFFFu, u32(f, kSdEntry0 + kE1MinorVer));
}

/* SWE5-SD-004 – SubscribeEventgroup produces entry type 0x06 */
TEST_F(SdTest, SWE5_SD_004_SubscribeEventgroupEntryType)
{
    ASSERT_EQ(E_OK, SomeIpSd_SubscribeEventgroup(0x0010, 0x0001, 0x0005, 0x01, 5));
    const uint8_t *f = StubTransport_GetLastFrame();
    EXPECT_EQ(0x06u, f[kSdEntry0 + kE1Type]);
    EXPECT_EQ(0x0010u, u16(f, kSdEntry0 + kE1ServiceId));
    EXPECT_EQ(0x0001u, u16(f, kSdEntry0 + kE1InstanceId));
    EXPECT_EQ(0x01u,   f[kSdEntry0 + kE1MajorVer]);
    EXPECT_EQ(0x0005u, u16(f, kSdEntry0 + kE2EventgroupId));
}

/* SWE5-SD-005 – Unsubscribe: SubscribeEventgroup with TTL=0 */
TEST_F(SdTest, SWE5_SD_005_UnsubscribeEventgroupTTLIsZero)
{
    ASSERT_EQ(E_OK, SomeIpSd_SubscribeEventgroup(0x0010, 0x0001, 0x0005, 0x01, 0));
    const uint8_t *f = StubTransport_GetLastFrame();
    EXPECT_EQ(0x00u, f[kSdEntry0 + kE1TTL_hi]);
    EXPECT_EQ(0x00u, f[kSdEntry0 + kE1TTL_hi + 1]);
    EXPECT_EQ(0x00u, f[kSdEntry0 + kE1TTL_hi + 2]);
}

/* SWE5-SD-006 – RxIndication accepts a well-formed SD frame */
TEST_F(SdTest, SWE5_SD_006_RxIndicationAcceptsValidFrame)
{
    /* First generate a real SD frame via OfferService */
    ASSERT_EQ(E_OK, SomeIpSd_OfferService(0x1234, 0x0001, 0x01, 1, 3));
    const uint8_t *frame = StubTransport_GetLastFrame();
    uint32_t       flen  = StubTransport_GetLastFrameLen();

    /* Feed it back into RxIndication */
    EXPECT_EQ(E_OK, SomeIpSd_RxIndication(frame, flen));
}

/* SWE5-SD-007 – RxIndication rejects non-SD service/method IDs (SR-SOMEIP-026) */
TEST_F(SdTest, SWE5_SD_007_RxIndicationRejectsNonSdFrame)
{
    /* Build a regular REQUEST frame (service != 0xFFFF) */
    SomeIp_Message_t msg{};
    msg.header.service_id        = 0x1234;
    msg.header.method_id         = 0x0001;
    msg.header.client_id         = 0x0001;
    msg.header.session_id        = 0x0001;
    msg.header.protocol_version  = SOMEIP_PROTOCOL_VERSION;
    msg.header.interface_version = SOMEIP_INTERFACE_VERSION;
    msg.header.message_type      = static_cast<uint8_t>(SOMEIP_MSG_REQUEST);
    msg.header.return_code       = static_cast<uint8_t>(SOMEIP_RC_OK);
    msg.payload                  = nullptr;
    msg.payload_length           = 0;

    uint8_t  buf[32]{};
    uint32_t len = 0;
    ASSERT_EQ(E_OK, SomeIp_Serialize(&msg, buf, sizeof(buf), &len));

    EXPECT_EQ(E_NOT_OK, SomeIpSd_RxIndication(buf, len));
}

/* SWE5-SD-008 – Transport failure propagated to caller (SR-SOMEIP-030) */
TEST_F(SdTest, SWE5_SD_008_TransportFailurePropagated)
{
    StubTransport_SetReturnValue(E_NOT_OK);
    EXPECT_EQ(E_NOT_OK, SomeIpSd_OfferService(0x0001, 0x0001, 0x01, 1, 3));
}

/* SWE5-SD-009 – RxIndication rejects NULL buffer */
TEST_F(SdTest, SWE5_SD_009_RxIndicationNullBuffer)
{
    EXPECT_EQ(E_NOT_OK, SomeIpSd_RxIndication(nullptr, 32));
}

/* SWE5-SD-010 – RxIndication rejects payload shorter than 8 bytes */
TEST_F(SdTest, SWE5_SD_010_RxIndicationPayloadTooShort)
{
    /* Build minimal SD-looking 16-byte header with tiny payload */
    uint8_t bad[20]{};
    bad[0] = 0xFF; bad[1] = 0xFF;  /* Service ID = 0xFFFF */
    bad[2] = 0x81; bad[3] = 0x00;  /* Method ID  = 0x8100 */
    bad[4] = 0x00; bad[5] = 0x00; bad[6] = 0x00; bad[7] = 0x08; /* Length = 8 (no payload) */
    bad[12] = 0x01; /* protocol version */
    bad[14] = 0x02; /* NOTIFICATION */
    bad[15] = 0x00; /* RC OK */

    /* length field = 8 means no payload → RxIndication sees payload_length = 0 */
    EXPECT_EQ(E_NOT_OK, SomeIpSd_RxIndication(bad, 16));
}

/* SWE5-SD-011 – Session ID increments on each SD send */
TEST_F(SdTest, SWE5_SD_011_SessionIdIncrements)
{
    ASSERT_EQ(E_OK, SomeIpSd_OfferService(0x0001, 0x0001, 0x01, 1, 3));
    uint16_t sess1 = u16(StubTransport_GetLastFrame(), 10);

    StubTransport_Reset();
    ASSERT_EQ(E_OK, SomeIpSd_OfferService(0x0001, 0x0001, 0x01, 1, 3));
    uint16_t sess2 = u16(StubTransport_GetLastFrame(), 10);

    EXPECT_EQ(sess1 + 1u, sess2);
}

/* SWE5-SD-012 – Reboot flag is set in SD flags byte (PRS_SOMEIPSD_00049) */
TEST_F(SdTest, SWE5_SD_012_RebootFlagSet)
{
    ASSERT_EQ(E_OK, SomeIpSd_OfferService(0x0001, 0x0001, 0x01, 1, 3));
    const uint8_t *f = StubTransport_GetLastFrame();
    EXPECT_TRUE(f[kSdFlags] & SOMEIP_SD_FLAG_REBOOT);
}

/* SWE5-SD-013 – Options array length field is 0 (no options attached) */
TEST_F(SdTest, SWE5_SD_013_OptionsArrayLengthIsZero)
{
    ASSERT_EQ(E_OK, SomeIpSd_OfferService(0x0001, 0x0001, 0x01, 1, 3));
    const uint8_t *f   = StubTransport_GetLastFrame();
    /* Options length field follows entries: offset = kSdEntry0 + 16 */
    size_t opts_off = kSdEntry0 + 16u;
    EXPECT_EQ(0u, u32(f, opts_off));
}
