#include <adas/SensorFusion.hpp>
#include <chrono>

namespace sdv::hpc::adas {

FusedFrame SensorFusion::fuse(const RawSensorInput& input)
{
    FusedFrame frame;
    frame.radarTargets = input.radarTargets;
    frame.lidarPoints  = input.lidarPoints;
    frame.camera       = input.camera;
    frame.timestampUs  = input.camera.timestampUs;
    return frame;
}

} // namespace sdv::hpc::adas
