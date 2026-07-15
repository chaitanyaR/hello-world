#include <body/LightingController.hpp>
#include <gtest/gtest.h>

using namespace sdv::zones::body;

TEST(LightingController, DefaultIsOff) {
    LightingController lc;
    EXPECT_EQ(lc.get(LightingZone::Headlights), LightingState::Off);
}

TEST(LightingController, SetAndGet) {
    LightingController lc;
    lc.set(LightingZone::Headlights, LightingState::On);
    EXPECT_EQ(lc.get(LightingZone::Headlights), LightingState::On);
}

TEST(LightingController, AllOff) {
    LightingController lc;
    lc.set(LightingZone::Headlights,     LightingState::On);
    lc.set(LightingZone::TailLights,     LightingState::Dim25);
    lc.set(LightingZone::InteriorFront,  LightingState::Dim50);
    lc.allOff();
    EXPECT_EQ(lc.get(LightingZone::Headlights),    LightingState::Off);
    EXPECT_EQ(lc.get(LightingZone::TailLights),    LightingState::Off);
    EXPECT_EQ(lc.get(LightingZone::InteriorFront), LightingState::Off);
}
