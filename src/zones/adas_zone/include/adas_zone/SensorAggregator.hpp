#pragma once
#include <adas_zone/RadarDriver.hpp>
#include <adas_zone/LidarDriver.hpp>
#include <adas_zone/CameraPreprocessor.hpp>
#include <cstdint>
#include <vector>
namespace sdv::zones::adas_zone {

struct SensorFrame {
    uint64_t                 timestampUs{};
    std::vector<RadarTarget> radarTargets;
    std::vector<LidarPoint>  lidarPoints;
    CameraFrameMeta          cameraFrame{};
};

class SensorAggregator {
public:
    SensorFrame aggregate();
};
}
