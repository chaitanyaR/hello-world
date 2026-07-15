#include <adas_zone/LidarDriver.hpp>

namespace sdv::zones::adas_zone {

std::vector<LidarPoint> LidarDriver::scan()
{
    std::vector<LidarPoint> pts;
    pts.reserve(9);
    for (int xi = -1; xi <= 1; ++xi) {
        for (int yi = -1; yi <= 1; ++yi) {
            pts.push_back({static_cast<float>(xi * 5),
                           static_cast<float>(yi * 5),
                           0.0f, 0.8f});
        }
    }
    return pts;
}

} // namespace sdv::zones::adas_zone
