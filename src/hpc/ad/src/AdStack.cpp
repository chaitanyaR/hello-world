#include <ad/AdStack.hpp>
namespace sdv::hpc::ad {
AdStack::AdStack(std::shared_ptr<ILocalizer> loc,
                 std::shared_ptr<IMapManager> map,
                 std::shared_ptr<IPredictionEngine> pred,
                 std::shared_ptr<IBehaviorPlanner> beh,
                 std::shared_ptr<ITrajectoryGenerator> traj)
    : localizer_(std::move(loc)), mapManager_(std::move(map)),
      prediction_(std::move(pred)), behavior_(std::move(beh)),
      trajectory_(std::move(traj))
{}
void AdStack::tick()
{
    auto pose  = localizer_->currentPose();
    auto tile  = mapManager_->tileAt(pose);
    auto wpts  = behavior_->plan(pose, tile);
    trajectory_->generate(pose, wpts);
}
}
