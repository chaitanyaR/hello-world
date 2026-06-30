#include <body/HvacController.hpp>
#include <gtest/gtest.h>

using namespace sdv::zones::body;

TEST(HvacController, ApplyAndReadBack) {
    HvacController hvac;
    HvacCommand cmd{22.0f, 20.0f, true, 50u};
    hvac.apply(cmd);
    HvacState s = hvac.currentState();
    EXPECT_FLOAT_EQ(s.driverActualCelsius,    22.0f);
    EXPECT_FLOAT_EQ(s.passengerActualCelsius, 20.0f);
    EXPECT_TRUE(s.acRunning);
    EXPECT_EQ(s.fanSpeedPercent, 50u);
}

TEST(HvacController, DefaultStateIsZero) {
    HvacController hvac;
    HvacState s = hvac.currentState();
    EXPECT_FLOAT_EQ(s.driverActualCelsius, 0.0f);
    EXPECT_FALSE(s.acRunning);
}
