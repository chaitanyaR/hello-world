#include <adas/ObjectDetector.hpp>

namespace sdv::hpc::adas {

DetectionList ObjectDetector::detect(const FusedFrame& frame)
{
    // Phase 4: real detection algorithm
    DetectionList result;
    for (const auto& target : frame.radarTargets) {
        DetectedObject obj;
        static uint32_t id = 1;
        obj.id         = id++;
        obj.x          = target.rangeM;
        obj.y          = 0.0f;
        obj.z          = 0.0f;
        obj.width      = 2.0f;
        obj.length     = 4.5f;
        obj.confidence = 0.8f;
        obj.classLabel = "vehicle";
        result.push_back(obj);
    }
    return result;
}

} // namespace sdv::hpc::adas
