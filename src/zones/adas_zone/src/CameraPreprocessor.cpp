#include <adas_zone/CameraPreprocessor.hpp>
#include <chrono>

namespace sdv::zones::adas_zone {

CameraFrameMeta CameraPreprocessor::capture()
{
    using namespace std::chrono;
    auto now = duration_cast<microseconds>(
        steady_clock::now().time_since_epoch()).count();
    return {1920u, 1080u, static_cast<uint64_t>(now)};
}

} // namespace sdv::zones::adas_zone
