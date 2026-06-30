#pragma once
#include <cstdint>
namespace sdv::zones::adas_zone {
struct CameraFrameMeta { uint32_t width; uint32_t height; uint64_t timestampUs; };
class CameraPreprocessor {
public:
    CameraFrameMeta capture();
};
}
