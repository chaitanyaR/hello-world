#pragma once

#include "Localizer.hpp"
#include <string>
#include <vector>

namespace sdv::hpc::ad {

struct LaneBoundary {
    std::vector<std::pair<float, float>> points;  // (x,y) in local frame
};

struct MapTile {
    std::vector<LaneBoundary> lanes;
    float speedLimitMps;
};

class IMapManager {
public:
    virtual ~IMapManager() = default;
    virtual MapTile tileAt(const Pose& pose) = 0;
};

// OpenDRIVE tile loader with LRU cache.
class MapManager : public IMapManager {
public:
    explicit MapManager(std::string mapPath);
    MapTile tileAt(const Pose& pose) override;
private:
    std::string mapPath_;
};

} // namespace sdv::hpc::ad
