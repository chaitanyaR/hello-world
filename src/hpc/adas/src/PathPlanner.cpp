#include <adas/PathPlanner.hpp>

namespace sdv::hpc::adas {

Path PathPlanner::plan(const EgoState& ego, const TrackList& /*obstacles*/)
{
    // Phase 4: RRT* implementation
    Path path;
    for (int i = 1; i <= 10; ++i) {
        Waypoint wp;
        wp.x = ego.x + i * 5.0f;
        wp.y = ego.y;
        wp.speedLimitMps = 13.9f; // 50 km/h
        path.push_back(wp);
    }
    return path;
}

} // namespace sdv::hpc::adas
