#include <adas_zone/SensorAggregator.hpp>
#include <adas_zone/RadarDriver.hpp>
#include <adas_zone/LidarDriver.hpp>
#include <adas_zone/CameraPreprocessor.hpp>
#include <chrono>

namespace sdv::zones::adas_zone {

SensorFrame SensorAggregator::aggregate()
{
    using namespace std::chrono;
    auto now = static_cast<uint64_t>(
        duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count());

    RadarDriver      radar;
    LidarDriver      lidar;
    CameraPreprocessor cam;

    SensorFrame frame;
    frame.timestampUs   = now;
    frame.radarTargets  = radar.scan();
    frame.lidarPoints   = lidar.scan();
    frame.cameraFrame   = cam.capture();
    return frame;
}

} // namespace sdv::zones::adas_zone
