#pragma once

#include "Localizer.hpp"
#include "MapManager.hpp"
#include "TrajectoryGenerator.hpp"
#include <vector>

namespace sdv::hpc::ad {

enum class BehaviorState { LaneKeep, LaneChange, Overtake, Stop, EmergencyBrake };

class IBehaviorPlanner {
public:
    virtual ~IBehaviorPlanner() = default;
    virtual std::vector<TrajectoryPoint> plan(const Pose& ego,
                                               const MapTile& map) = 0;
    virtual BehaviorState currentBehavior() const = 0;
};

// Behavior Tree based planner (BT.CPP integration in Phase 5).
class BehaviorPlanner : public IBehaviorPlanner {
public:
    std::vector<TrajectoryPoint> plan(const Pose& ego,
                                      const MapTile& map) override;
    BehaviorState currentBehavior() const override;
private:
    BehaviorState behavior_{BehaviorState::LaneKeep};
};

} // namespace sdv::hpc::ad
