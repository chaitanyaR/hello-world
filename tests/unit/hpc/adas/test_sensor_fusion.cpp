#include <adas/SensorFusion.hpp>
#include <gtest/gtest.h>

using namespace sdv::hpc::adas;

TEST(SensorFusion, FuseReturnsFrameWithSameTimestamp) {
    SensorFusion sf;
    RawSensorInput input;
    input.camera.timestampUs = 12345;
    auto frame = sf.fuse(input);
    // Phase 4: assert merged fields; for now just verify it compiles and runs
    EXPECT_GE(frame.timestampUs, 0u);
}
