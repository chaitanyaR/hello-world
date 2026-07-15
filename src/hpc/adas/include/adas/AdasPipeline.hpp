#pragma once

#include "SensorFusion.hpp"
#include "ObjectDetector.hpp"
#include "ObjectTracker.hpp"
#include "PathPlanner.hpp"
#include "DecisionMaker.hpp"
#include "SafetyMonitor.hpp"
#include <memory>
#include <atomic>
#include <thread>

namespace sdv::hpc::adas {

// Orchestrates the ADAS processing pipeline at a fixed tick rate (default 50 Hz).
// Data flows: SensorFusion → ObjectDetector → ObjectTracker → PathPlanner → DecisionMaker
//
// Each stage implements IPipelineStage<Input,Output> for independent testability.
// The pipeline runs deterministically in a single thread to satisfy ASIL-D requirements.
class AdasPipeline {
public:
    struct Config {
        unsigned tickRateHz{50};
    };

    AdasPipeline(Config cfg,
                 std::shared_ptr<ISensorFusion>   fusion,
                 std::shared_ptr<IObjectDetector> detector,
                 std::shared_ptr<IObjectTracker>  tracker,
                 std::shared_ptr<IPathPlanner>    planner,
                 std::shared_ptr<IDecisionMaker>  decision,
                 std::shared_ptr<ISafetyMonitor>  safety);

    ~AdasPipeline();

    void start();
    void stop();

    bool isRunning() const;

private:
    void tickLoop();

    Config                           config_;
    std::shared_ptr<ISensorFusion>   fusion_;
    std::shared_ptr<IObjectDetector> detector_;
    std::shared_ptr<IObjectTracker>  tracker_;
    std::shared_ptr<IPathPlanner>    planner_;
    std::shared_ptr<IDecisionMaker>  decision_;
    std::shared_ptr<ISafetyMonitor>  safety_;

    std::atomic<bool> running_{false};
    std::thread       tickThread_;
};

} // namespace sdv::hpc::adas
