#pragma once
#include <vector>
namespace sdv::zones::adas_zone {
struct RadarTarget { float rangeM; float azimuthDeg; float velocityMps; };
class RadarDriver {
public:
    std::vector<RadarTarget> scan();
};
}
