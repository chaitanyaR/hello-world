#pragma once
#include <vector>
namespace sdv::zones::adas_zone {
struct LidarPoint { float x; float y; float z; float intensity; };
class LidarDriver {
public:
    std::vector<LidarPoint> scan();
};
}
