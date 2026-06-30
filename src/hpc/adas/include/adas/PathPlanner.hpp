#pragma once

#include "ObjectTracker.hpp"
#include <vector>

namespace sdv::hpc::adas {

struct Waypoint {
    float x, y;          // vehicle frame (m)
    float speedLimitMps;
};

using Path = std::vector<Waypoint>;

struct EgoState {
    float x, y, heading; // vehicle pose
    float speedMps;
};

class IPathPlanner {
public:
    virtual ~IPathPlanner() = default;
    virtual Path plan(const EgoState& ego, const TrackList& obstacles) = 0;
};

// Geometric path planner (RRT* stub; upgrade to full RRT* in Phase 6).
class PathPlanner : public IPathPlanner {
public:
    Path plan(const EgoState& ego, const TrackList& obstacles) override;
};

} // namespace sdv::hpc::adas
