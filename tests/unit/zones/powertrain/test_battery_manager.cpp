#include <powertrain/BatteryManager.hpp>
#include <gtest/gtest.h>

using namespace sdv::zones::powertrain;

TEST(BatteryManager, StartsIdle) {
    BatteryManager bm;
    EXPECT_EQ(bm.fsmState(), BatteryFsmState::Idle);
}

TEST(BatteryManager, TransitionsToDischargingOnLoad) {
    BatteryManager bm;
    bm.update(0.0f, 10.0f, 25.0f); // load current > 5A, no charger
    EXPECT_EQ(bm.fsmState(), BatteryFsmState::Discharging);
}

TEST(BatteryManager, TransitionsToChargingOnCharger) {
    BatteryManager bm;
    bm.update(400.0f, 0.0f, 25.0f); // charger voltage > 100V
    EXPECT_EQ(bm.fsmState(), BatteryFsmState::Charging);
}

TEST(BatteryManager, FaultOnOvertemperature) {
    BatteryManager bm;
    bm.update(0.0f, 10.0f, 60.0f); // temp > 55°C
    EXPECT_EQ(bm.fsmState(), BatteryFsmState::Fault);
}

TEST(BatteryManager, FaultOnOvercurrent) {
    BatteryManager bm;
    bm.update(0.0f, 350.0f, 25.0f); // current > 300A
    EXPECT_EQ(bm.fsmState(), BatteryFsmState::Fault);
}
