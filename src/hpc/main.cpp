#include <adas/AdasPipeline.hpp>
#include <ad/AdStack.hpp>
#include <sovd/SovdServer.hpp>
#include <someip/ServiceRegistry.hpp>
#include <someip/SomeIpConfig.hpp>
#include <csignal>
#include <atomic>
#include <iostream>

static std::atomic<bool> g_running{true};

int main() {
    std::signal(SIGINT,  [](int) { g_running = false; });
    std::signal(SIGTERM, [](int) { g_running = false; });

    sdv::someip::SomeIpConfig::load("config/someip/hpc_vsomeip.json");
    sdv::someip::ServiceRegistry registry("sdv_hpc");

    sdv::sovd::SovdServerConfig sovdCfg;
    sovdCfg.port = 8080;
    sdv::sovd::SovdServer sovdServer(sovdCfg);
    // TODO Phase 3: mount all SOVD resources
    sovdServer.start();

    // TODO Phase 4: instantiate and start AdasPipeline
    // TODO Phase 7: instantiate and start AdStack

    registry.start();
    std::cout << "[HPC] Running. Press Ctrl+C to stop.\n";
    while (g_running) { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }

    sovdServer.stop();
    registry.stop();
    return 0;
}
