#include <ad/Localizer.hpp>
#include <gtest/gtest.h>
TEST(Localizer, InitialPoseIsZero) {
    sdv::hpc::ad::Localizer loc;
    auto pose = loc.currentPose();
    EXPECT_DOUBLE_EQ(pose.latitudeDeg, 0.0);
}
