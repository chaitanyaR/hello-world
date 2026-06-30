#pragma once

#include "Localizer.hpp"
#include "MapManager.hpp"
#include "TrajectoryGenerator.hpp"
#include "BehaviorPlanner.hpp"
#include "PredictionEngine.hpp"
#include <memory>

namespace sdv::hpc::ad {

// Orchestrates the full Autonomous Driving stack:
//   Localizer → MapManager → PredictionEngine → BehaviorPlanner → TrajectoryGenerator
class AdStack {
public:
    AdStack(std::shared_ptr<ILocalizer>            localizer,
            std::shared_ptr<IMapManager>           mapManager,
            std::shared_ptr<IPredictionEngine>     prediction,
            std::shared_ptr<IBehaviorPlanner>      behavior,
            std::shared_ptr<ITrajectoryGenerator>  trajectory);

    void tick();

private:
    std::shared_ptr<ILocalizer>           localizer_;
    std::shared_ptr<IMapManager>          mapManager_;
    std::shared_ptr<IPredictionEngine>    prediction_;
    std::shared_ptr<IBehaviorPlanner>     behavior_;
    std::shared_ptr<ITrajectoryGenerator> trajectory_;
};

} // namespace sdv::hpc::ad
