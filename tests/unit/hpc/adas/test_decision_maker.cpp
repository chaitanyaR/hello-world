#include <adas/DecisionMaker.hpp>
#include <gtest/gtest.h>

using namespace sdv::hpc::adas;

TEST(DecisionMaker, StartsInNominalState) {
    DecisionMaker dm;
    EXPECT_EQ(dm.currentState(), DrivingState::Nominal);
}
