#include <adas_zone/SensorAggregator.hpp>
#include <adas_zone/ActuatorController.hpp>
#include <adas_zone/SafetyGateway.hpp>
#include <powertrain/TorqueArbiter.hpp>
#include <powertrain/BatteryManager.hpp>
#include <telemetry/OtaManager.hpp>
#include <body/HvacController.hpp>
#include <body/LightingController.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

// In-process integration test: exercises all zone ECU components together
// without SOME/IP (simulation mode) to verify the full data flow.

TEST(FullPipeline, SensorAggregatorToActuator) {
    sdv::zones::adas_zone::ActuatorController ctrl;
    sdv::zones::adas_zone::SafetyGateway      gw(ctrl);
    sdv::zones::adas_zone::SensorAggregator   agg;

    auto frame = agg.aggregate();
    EXPECT_GT(frame.radarTargets.size(), 0u);
    EXPECT_GT(frame.lidarPoints.size(), 0u);
    EXPECT_GT(frame.cameraFrame.width, 0u);

    sdv::zones::adas_zone::ActuatorCommand cmd{5.0f, 20.0f, 10.0f, false, frame.timestampUs};
    EXPECT_TRUE(gw.applyCommand(cmd));
    EXPECT_FALSE(gw.isViolation());
    EXPECT_FLOAT_EQ(ctrl.lastApplied().throttlePercent, 20.0f);
}

TEST(FullPipeline, BatteryManagerDischargesUnderLoad) {
    sdv::zones::powertrain::BatteryManager bm;
    EXPECT_EQ(bm.fsmState(), sdv::zones::powertrain::BatteryFsmState::Idle);

    bm.update(0.0f, 20.0f, 30.0f);
    EXPECT_EQ(bm.fsmState(), sdv::zones::powertrain::BatteryFsmState::Discharging);
}

TEST(FullPipeline, TorqueArbiterIntegration) {
    sdv::zones::powertrain::TorqueArbiter arb;
    arb.submit({150.0f, 2u, 0u});
    arb.submit({100.0f, 1u, 0u}); // higher priority
    float granted = arb.arbitrate(200.0f);
    EXPECT_FLOAT_EQ(granted, 100.0f);
}

TEST(FullPipeline, OtaManagerReachesVerifying) {
    using sdv::zones::telemetry::OtaState;
    std::atomic<OtaState> lastState{OtaState::Idle};

    sdv::zones::telemetry::OtaManager ota([&](OtaState s, uint8_t) {
        lastState = s;
    });

    bool triggered = ota.trigger("http://update.example.com/pkg.bin", "deadbeef");
    EXPECT_TRUE(triggered);

    // Wait for OTA FSM to advance beyond Downloading
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (std::chrono::steady_clock::now() < deadline) {
        OtaState s = lastState.load();
        if (s == OtaState::Verifying || s == OtaState::Installing || s == OtaState::Done)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    OtaState final = ota.currentState();
    EXPECT_NE(final, OtaState::Idle);
    EXPECT_NE(final, OtaState::Failed);
}

TEST(FullPipeline, BodyControllersWork) {
    sdv::zones::body::HvacController hvac;
    hvac.apply({21.0f, 19.0f, true, 40u});
    EXPECT_FLOAT_EQ(hvac.currentState().driverActualCelsius, 21.0f);

    sdv::zones::body::LightingController lighting;
    lighting.set(sdv::zones::body::LightingZone::Headlights,
                 sdv::zones::body::LightingState::On);
    EXPECT_EQ(lighting.get(sdv::zones::body::LightingZone::Headlights),
              sdv::zones::body::LightingState::On);
    lighting.allOff();
    EXPECT_EQ(lighting.get(sdv::zones::body::LightingZone::Headlights),
              sdv::zones::body::LightingState::Off);
}
