#pragma once

#include "SensorFusion.hpp"
#include <string>
#include <vector>

namespace sdv::hpc::adas {

struct DetectedObject {
    uint32_t    id;
    float       x, y, z;        // position in vehicle frame (m)
    float       width, length;  // bounding box (m)
    float       confidence;
    std::string classLabel;      // "vehicle" | "pedestrian" | "cyclist" | "obstacle"
};

using DetectionList = std::vector<DetectedObject>;

class IObjectDetector {
public:
    virtual ~IObjectDetector() = default;
    virtual DetectionList detect(const FusedFrame& frame) = 0;
};

class ObjectDetector : public IObjectDetector {
public:
    DetectionList detect(const FusedFrame& frame) override;
};

} // namespace sdv::hpc::adas
