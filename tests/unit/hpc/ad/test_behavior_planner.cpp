#include <ad/BehaviorPlanner.hpp>
#include <gtest/gtest.h>

using namespace sdv::hpc::ad;

TEST(BehaviorPlanner, LaneKeepWhenMoving) {
    BehaviorPlanner bp;
    Pose ego{};
    ego.speedMps = 10.0f;
    MapTile map{};
    auto path = bp.plan(ego, map);
    EXPECT_EQ(bp.currentBehavior(), BehaviorState::LaneKeep);
    EXPECT_EQ(path.size(), 5u);
}

TEST(BehaviorPlanner, StopWhenStationary) {
    BehaviorPlanner bp;
    Pose ego{};
    ego.speedMps = 0.0f;
    MapTile map{};
    bp.plan(ego, map);
    EXPECT_EQ(bp.currentBehavior(), BehaviorState::Stop);
}

TEST(BehaviorPlanner, PathHasCorrectSize) {
    BehaviorPlanner bp;
    Pose ego{};
    ego.speedMps = 5.0f;
    MapTile map{};
    auto path = bp.plan(ego, map);
    EXPECT_FALSE(path.empty());
}
