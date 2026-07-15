#include <telemetry/TelemetryPublisher.hpp>
#include <telemetry/OtaManager.hpp>
#include <telemetry/ConnectivityManager.hpp>
#include <telemetry/TelemetryServiceStub.hpp>
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

    sdv::someip::SomeIpConfig::load("config/someip/telemetry_vsomeip.json");
    sdv::someip::ServiceRegistry registry("sdv_telemetry");

    sdv::zones::telemetry::TelemetryPublisher  publisher;
    sdv::zones::telemetry::ConnectivityManager connectivity;
    sdv::zones::telemetry::OtaManager ota([](sdv::zones::telemetry::OtaState s, uint8_t p) {
        std::cout << "[Telemetry] OTA state=" << static_cast<int>(s)
                  << " progress=" << static_cast<int>(p) << "%\n";
    });
    sdv::zones::telemetry::TelemetryServiceStub stub(registry);

    connectivity.setSignalStrength(87);
    stub.offer();
    registry.start();
    std::cout << "[Telemetry ECU] Running.\n";

    while (g_running) {
        publisher.publish();
        std::this_thread::sleep_for(100ms);
    }

    stub.withdraw();
    registry.stop();
}
