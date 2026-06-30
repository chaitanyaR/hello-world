#include <ad/BehaviorPlanner.hpp>
namespace sdv::hpc::ad {
std::vector<TrajectoryPoint> BehaviorPlanner::plan(const Pose&, const MapTile&)
{
    return {};
}
BehaviorState BehaviorPlanner::currentBehavior() const { return behavior_; }
}
