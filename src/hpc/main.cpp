#include <adas/AdasPipeline.hpp>
#include <adas/SensorFusion.hpp>
#include <adas/ObjectDetector.hpp>
#include <adas/ObjectTracker.hpp>
#include <adas/PathPlanner.hpp>
#include <adas/DecisionMaker.hpp>
#include <adas/SafetyMonitor.hpp>
#include <ad/AdStack.hpp>
#include <ad/Localizer.hpp>
#include <ad/MapManager.hpp>
#include <ad/PredictionEngine.hpp>
#include <ad/BehaviorPlanner.hpp>
#include <ad/TrajectoryGenerator.hpp>
#include <sovd/SovdServer.hpp>
#include <someip/ServiceRegistry.hpp>
#include <someip/SomeIpConfig.hpp>
#include <sovd_server/HpcSovdResource.hpp>
#include <sovd_server/AdasSovdResource.hpp>
#include <sovd_server/AdSovdResource.hpp>
#include <sovd_server/ZoneForwarder.hpp>
#include <csignal>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

using namespace std::chrono_literals;

static std::atomic<bool> g_running{true};

int main() {
    std::signal(SIGINT,  [](int) { g_running = false; });
    std::signal(SIGTERM, [](int) { g_running = false; });

    sdv::someip::SomeIpConfig::load("config/someip/hpc_vsomeip.json");
    sdv::someip::ServiceRegistry registry("sdv_hpc");

    // SOVD server setup
    sdv::sovd::SovdServerConfig sovdCfg;
    sovdCfg.port = 8080;
    sdv::sovd::SovdServer sovdServer(sovdCfg);

    // Mount HPC-level SOVD resources
    sovdServer.router().mount("/api/sovd/v1/hpc",
        std::make_shared<sdv::hpc::sovd_server::HpcSovdResource>());
    sovdServer.router().mount("/api/sovd/v1/hpc/adas",
        std::make_shared<sdv::hpc::sovd_server::AdasSovdResource>());
    sovdServer.router().mount("/api/sovd/v1/hpc/ad",
        std::make_shared<sdv::hpc::sovd_server::AdSovdResource>());

    // Mount zone SOVD forwarders (one per zone ECU)
    sovdServer.router().mount("/api/sovd/v1/zones/powertrain",
        std::make_shared<sdv::hpc::sovd_server::ZoneForwarder>(
            "powertrain", 0x1FFFu, 0x0001u, registry));
    sovdServer.router().mount("/api/sovd/v1/zones/adas",
        std::make_shared<sdv::hpc::sovd_server::ZoneForwarder>(
            "adas", 0x1FFFu, 0x0002u, registry));
    sovdServer.router().mount("/api/sovd/v1/zones/telemetry",
        std::make_shared<sdv::hpc::sovd_server::ZoneForwarder>(
            "telemetry", 0x1FFFu, 0x0003u, registry));
    sovdServer.router().mount("/api/sovd/v1/zones/body",
        std::make_shared<sdv::hpc::sovd_server::ZoneForwarder>(
            "body", 0x1FFFu, 0x0004u, registry));

    sovdServer.start();

    // ADAS pipeline — inject all stages
    auto pipeline = std::make_unique<sdv::hpc::adas::AdasPipeline>(
        sdv::hpc::adas::AdasPipeline::Config{50u},
        std::make_shared<sdv::hpc::adas::SensorFusion>(),
        std::make_shared<sdv::hpc::adas::ObjectDetector>(),
        std::make_shared<sdv::hpc::adas::ObjectTracker>(),
        std::make_shared<sdv::hpc::adas::PathPlanner>(),
        std::make_shared<sdv::hpc::adas::DecisionMaker>(),
        std::make_shared<sdv::hpc::adas::SafetyMonitor>(
            std::chrono::milliseconds{100}, [] {
                std::cerr << "[HPC] SAFE STATE triggered by watchdog\n";
            })
    );
    pipeline->start();

    // AD stack — tick at 10 Hz in a background thread
    auto adStack = std::make_unique<sdv::hpc::ad::AdStack>(
        std::make_shared<sdv::hpc::ad::Localizer>(),
        std::make_shared<sdv::hpc::ad::MapManager>(""),
        std::make_shared<sdv::hpc::ad::PredictionEngine>(),
        std::make_shared<sdv::hpc::ad::BehaviorPlanner>(),
        std::make_shared<sdv::hpc::ad::TrajectoryGenerator>()
    );
    std::atomic<bool> adRunning{true};
    std::thread adThread([&] {
        while (adRunning) {
            adStack->tick();
            std::this_thread::sleep_for(100ms);
        }
    });

    registry.start();
    std::cout << "[HPC] Running. Press Ctrl+C to stop.\n";
    while (g_running) { std::this_thread::sleep_for(100ms); }

    adRunning = false;
    adThread.join();
    pipeline->stop();
    sovdServer.stop();
    registry.stop();
    return 0;
}
