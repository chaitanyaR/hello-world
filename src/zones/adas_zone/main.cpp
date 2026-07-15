#include <adas_zone/SensorAggregator.hpp>
#include <adas_zone/ActuatorController.hpp>
#include <adas_zone/SafetyGateway.hpp>
#include <adas_zone/AdasZoneServiceStub.hpp>
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

    sdv::someip::SomeIpConfig::load("config/someip/adas_vsomeip.json");
    sdv::someip::ServiceRegistry registry("sdv_adas_zone");

    sdv::zones::adas_zone::ActuatorController  actuator;
    sdv::zones::adas_zone::SafetyGateway       gateway(actuator);
    sdv::zones::adas_zone::SensorAggregator    aggregator;
    sdv::zones::adas_zone::AdasZoneServiceStub stub(registry);

    stub.offer();
    registry.start();
    std::cout << "[ADAS Zone ECU] Running.\n";

    while (g_running) {
        auto frame = aggregator.aggregate();
        (void)frame; // frame would be sent via SOME/IP event in real system
        std::this_thread::sleep_for(20ms); // 50 Hz
    }

    stub.withdraw();
    registry.stop();
}
