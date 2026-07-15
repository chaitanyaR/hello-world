#include <adas/ObjectTracker.hpp>

namespace sdv::hpc::adas {

TrackList ObjectTracker::update(const DetectionList& detections)
{
    // Phase 4: Kalman filter + Hungarian assignment
    // For now: promote all detections directly to tracks (no association)
    TrackList result;
    for (const auto& det : detections) {
        TrackedObject t;
        t.id         = nextId_++;
        t.x          = det.x;
        t.y          = det.y;
        t.z          = det.z;
        t.vx         = 0.0f;
        t.vy         = 0.0f;
        t.confidence = det.confidence;
        t.age        = 1;
        result.push_back(t);
    }
    return result;
}

} // namespace sdv::hpc::adas
