#include <powertrain/TorqueArbiter.hpp>
#include <gtest/gtest.h>

using namespace sdv::zones::powertrain;

TEST(TorqueArbiter, NoRequestsReturnsLastGranted) {
    TorqueArbiter arb;
    EXPECT_FLOAT_EQ(arb.arbitrate(300.0f), 0.0f);
}

TEST(TorqueArbiter, HighestPriorityWins) {
    TorqueArbiter arb;
    arb.submit({100.0f, 3u, 0u});
    arb.submit({200.0f, 1u, 0u}); // priority 1 wins (lower value = higher priority)
    arb.submit({50.0f,  2u, 0u});
    float granted = arb.arbitrate(300.0f);
    EXPECT_FLOAT_EQ(granted, 200.0f);
}

TEST(TorqueArbiter, CapAtMaxCapability) {
    TorqueArbiter arb;
    arb.submit({500.0f, 0u, 0u});
    EXPECT_FLOAT_EQ(arb.arbitrate(250.0f), 250.0f);
}

TEST(TorqueArbiter, LastGrantedPersists) {
    TorqueArbiter arb;
    arb.submit({80.0f, 1u, 0u});
    arb.arbitrate(300.0f);
    EXPECT_FLOAT_EQ(arb.lastGrantedNm(), 80.0f);
}
