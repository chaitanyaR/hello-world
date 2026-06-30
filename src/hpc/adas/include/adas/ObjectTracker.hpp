#pragma once

#include "ObjectDetector.hpp"
#include <vector>

namespace sdv::hpc::adas {

struct TrackedObject {
    uint32_t id;
    float    x, y, z;
    float    vx, vy;     // velocity (m/s) in vehicle frame
    float    confidence;
    uint32_t age;        // frames since first detection
};

using TrackList = std::vector<TrackedObject>;

class IObjectTracker {
public:
    virtual ~IObjectTracker() = default;
    // Associate detections with existing tracks; return updated track list.
    virtual TrackList update(const DetectionList& detections) = 0;
};

// Kalman-filter multi-object tracker with Hungarian assignment.
class ObjectTracker : public IObjectTracker {
public:
    TrackList update(const DetectionList& detections) override;
private:
    TrackList   tracks_;
    uint32_t    nextId_{1};
};

} // namespace sdv::hpc::adas
