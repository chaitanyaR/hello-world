#include <someip/ServiceRegistry.hpp>
#include <someip/SomeIpConfig.hpp>
#include <csignal>
#include <atomic>
#include <iostream>
#include <thread>
#include <chrono>

static std::atomic<bool> g_running{true};
int main() {
    std::signal(SIGINT,  [](int) { g_running = false; });
    std::signal(SIGTERM, [](int) { g_running = false; });
    sdv::someip::SomeIpConfig::load("config/someip/telemetry_vsomeip.json");
    sdv::someip::ServiceRegistry registry("sdv_telemetry");
    registry.start();
    std::cout << "[Telemetry ECU] Running.\n";
    while (g_running) { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
    registry.stop();
}
