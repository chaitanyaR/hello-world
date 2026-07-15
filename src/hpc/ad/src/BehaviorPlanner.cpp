#include <ad/BehaviorPlanner.hpp>
#include <cmath>

namespace sdv::hpc::ad {

std::vector<TrajectoryPoint> BehaviorPlanner::plan(const Pose& ego,
                                                     const MapTile& /*map*/)
{
    // Simplified FSM: stay in LaneKeep unless speed is very low → Stop
    if (ego.speedMps < 0.1f) {
        behavior_ = BehaviorState::Stop;
    } else {
        behavior_ = BehaviorState::LaneKeep;
    }

    // Generate a straight-ahead reference path: 5 waypoints at 1s intervals
    std::vector<TrajectoryPoint> path;
    path.reserve(5);
    for (int i = 1; i <= 5; ++i) {
        TrajectoryPoint pt{};
        float dt = static_cast<float>(i);
        pt.x                = static_cast<float>(ego.longitudeDeg) + ego.speedMps * dt * 1e-5f;
        pt.y                = static_cast<float>(ego.latitudeDeg);
        pt.speedMps         = ego.speedMps;
        pt.headingDeg       = ego.headingDeg;
        pt.curvature        = 0.0f;
        pt.timestampOffsetS = dt;
        path.push_back(pt);
    }
    return path;
}

BehaviorState BehaviorPlanner::currentBehavior() const { return behavior_; }

} // namespace sdv::hpc::ad
