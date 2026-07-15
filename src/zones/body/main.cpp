#include <body/LightingController.hpp>
#include <body/DoorWindowController.hpp>
#include <body/HvacController.hpp>
#include <body/ComfortManager.hpp>
#include <body/BodyServiceStub.hpp>
#include <someip/ServiceRegistry.hpp>
#include <someip/SomeIpConfig.hpp>
#include <csignal>
#include <atomic>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;
static std::atomic<bool> g_running{true};

int main() {
    std::signal(SIGINT,  [](int) { g_running = false; });
    std::signal(SIGTERM, [](int) { g_running = false; });

    sdv::someip::SomeIpConfig::load("config/someip/body_vsomeip.json");
    sdv::someip::ServiceRegistry registry("sdv_body");

    sdv::zones::body::LightingController    lighting;
    sdv::zones::body::DoorWindowController  doors;
    sdv::zones::body::HvacController        hvac;
    sdv::zones::body::ComfortManager        comfort;
    sdv::zones::body::BodyServiceStub       stub(registry);

    // Initial state: headlights on, all doors locked, HVAC at 22°C
    lighting.set(sdv::zones::body::LightingZone::Headlights,
                 sdv::zones::body::LightingState::On);
    doors.setLock(sdv::zones::body::DoorId::FrontLeft,  true);
    doors.setLock(sdv::zones::body::DoorId::FrontRight, true);
    hvac.apply({22.0f, 22.0f, false, 30u});

    stub.offer();
    registry.start();
    std::cout << "[Body ECU] Running.\n";

    while (g_running) { std::this_thread::sleep_for(100ms); }

    stub.withdraw();
    registry.stop();
}
