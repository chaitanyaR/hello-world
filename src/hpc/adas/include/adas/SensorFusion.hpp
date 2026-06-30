#pragma once

#include <cstdint>
#include <vector>

namespace sdv::hpc::adas {

struct RadarTarget   { float rangeM; float azimuthDeg; float velocityMps; };
struct LidarPoint    { float x; float y; float z; float intensity; };
struct CameraFrame   { uint32_t width; uint32_t height; uint64_t timestampUs; };

struct FusedFrame {
    std::vector<RadarTarget> radarTargets;
    std::vector<LidarPoint>  lidarPoints;
    CameraFrame              camera;
    uint64_t                 timestampUs;
};

struct RawSensorInput {
    std::vector<RadarTarget> radarTargets;
    std::vector<LidarPoint>  lidarPoints;
    CameraFrame              camera;
};

class ISensorFusion {
public:
    virtual ~ISensorFusion() = default;
    virtual FusedFrame fuse(const RawSensorInput& input) = 0;
};

class SensorFusion : public ISensorFusion {
public:
    FusedFrame fuse(const RawSensorInput& input) override;
};

} // namespace sdv::hpc::adas
