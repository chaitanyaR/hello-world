#include <adas_zone/SafetyGateway.hpp>
#include <adas_zone/ActuatorController.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

using namespace sdv::zones::adas_zone;

TEST(SafetyGateway, ValidCommandPasses) {
    ActuatorController ctrl;
    SafetyGateway gw(ctrl);
    ActuatorCommand cmd{10.0f, 30.0f, 20.0f, false, 0u};
    EXPECT_TRUE(gw.applyCommand(cmd));
    EXPECT_FALSE(gw.isViolation());
}

TEST(SafetyGateway, ExcessiveSteeringAngleRejected) {
    ActuatorController ctrl;
    SafetyGateway gw(ctrl);
    ActuatorCommand cmd{600.0f, 0.0f, 0.0f, false, 0u}; // > 540° limit
    EXPECT_FALSE(gw.applyCommand(cmd));
    EXPECT_TRUE(gw.isViolation());
}

TEST(SafetyGateway, ExcessiveBrakePressureRejected) {
    ActuatorController ctrl;
    SafetyGateway gw(ctrl);
    ActuatorCommand cmd{0.0f, 0.0f, 200.0f, false, 0u}; // > 150 bar limit
    EXPECT_FALSE(gw.applyCommand(cmd));
    EXPECT_TRUE(gw.isViolation());
}

TEST(SafetyGateway, HoldLastValidOnViolation) {
    ActuatorController ctrl;
    SafetyGateway gw(ctrl);

    // First valid command
    ActuatorCommand valid{10.0f, 20.0f, 5.0f, false, 0u};
    gw.applyCommand(valid);

    // Now violate
    ActuatorCommand bad{0.0f, 0.0f, 200.0f, false, 0u};
    gw.applyCommand(bad);

    // Controller should have re-applied the last valid command
    EXPECT_FLOAT_EQ(ctrl.lastApplied().brakePressureBar, 5.0f);
}
