#include <adas_zone/RadarDriver.hpp>

namespace sdv::zones::adas_zone {

std::vector<RadarTarget> RadarDriver::scan()
{
    return {
        {15.0f,  -5.0f, -2.5f},
        {30.0f,   0.0f,  0.0f},
        {45.0f,  10.0f,  1.0f},
    };
}

} // namespace sdv::zones::adas_zone
