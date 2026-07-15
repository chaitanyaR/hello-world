#include <adas/AdasPipeline.hpp>
#include <thread>

namespace sdv::hpc::adas {

AdasPipeline::AdasPipeline(Config cfg,
                            std::shared_ptr<ISensorFusion>   fusion,
                            std::shared_ptr<IObjectDetector> detector,
                            std::shared_ptr<IObjectTracker>  tracker,
                            std::shared_ptr<IPathPlanner>    planner,
                            std::shared_ptr<IDecisionMaker>  decision,
                            std::shared_ptr<ISafetyMonitor>  safety)
    : config_(cfg), fusion_(std::move(fusion)), detector_(std::move(detector)),
      tracker_(std::move(tracker)), planner_(std::move(planner)),
      decision_(std::move(decision)), safety_(std::move(safety))
{}

AdasPipeline::~AdasPipeline() { stop(); }

void AdasPipeline::start()
{
    running_ = true;
    tickThread_ = std::thread([this] { tickLoop(); });
}

void AdasPipeline::stop()
{
    running_ = false;
    if (tickThread_.joinable()) tickThread_.join();
}

bool AdasPipeline::isRunning() const { return running_.load(); }

void AdasPipeline::tickLoop()
{
    using namespace std::chrono;
    const auto period = milliseconds(1000 / config_.tickRateHz);
    while (running_.load()) {
        auto tick_start = steady_clock::now();

        // Phase 4: pull RawSensorInput from SOME/IP event queue
        RawSensorInput raw{};
        auto fused    = fusion_->fuse(raw);
        auto detects  = detector_->detect(fused);
        auto tracks   = tracker_->update(detects);
        EgoState ego{};
        auto path     = planner_->plan(ego, tracks);
        decision_->decide(path, tracks, ego);
        safety_->heartbeat();

        auto elapsed = steady_clock::now() - tick_start;
        if (elapsed < period)
            std::this_thread::sleep_for(period - elapsed);
    }
}

} // namespace sdv::hpc::adas
