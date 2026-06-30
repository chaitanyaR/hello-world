#include <powertrain/TorqueArbiter.hpp>
#include <powertrain/EngineController.hpp>
#include <powertrain/BatteryManager.hpp>
#include <powertrain/RegenBraking.hpp>
#include <powertrain/PowertrainServiceStub.hpp>
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

    sdv::someip::SomeIpConfig::load("config/someip/powertrain_vsomeip.json");
    sdv::someip::ServiceRegistry registry("sdv_powertrain");

    sdv::zones::powertrain::TorqueArbiter   arbiter;
    sdv::zones::powertrain::EngineController engine;
    sdv::zones::powertrain::BatteryManager  battery;
    sdv::zones::powertrain::RegenBraking    regen;
    sdv::zones::powertrain::PowertrainServiceStub stub(registry);

    stub.offer();
    registry.start();
    std::cout << "[Powertrain ECU] Running.\n";

    while (g_running) {
        battery.update(0.0f, 10.0f, 25.0f);  // synthetic: discharging
        float torque = arbiter.arbitrate(300.0f);
        engine.setTorqueDemandNm(torque);
        std::this_thread::sleep_for(100ms);
    }

    stub.withdraw();
    registry.stop();
}
