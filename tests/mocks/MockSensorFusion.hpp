#pragma once
#include <adas/SensorFusion.hpp>
#include <gmock/gmock.h>

namespace sdv::test {
class MockSensorFusion : public hpc::adas::ISensorFusion {
public:
    MOCK_METHOD(hpc::adas::FusedFrame, fuse,
                (const hpc::adas::RawSensorInput&), (override));
};
}
