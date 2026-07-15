#include <adas/ObjectTracker.hpp>
#include <gtest/gtest.h>

using namespace sdv::hpc::adas;

TEST(ObjectTracker, EmptyDetectionsProducesNoTracks) {
    ObjectTracker tracker;
    auto tracks = tracker.update({});
    // Phase 4: will validate Kalman filter behavior
    EXPECT_EQ(tracks.size(), 0u);
}
