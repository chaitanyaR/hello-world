#pragma once

#include "Localizer.hpp"
#include <vector>

namespace sdv::hpc::ad {

struct TrajectoryPoint {
    float x, y;
    float speedMps;
    float headingDeg;
    float curvature;
    float timestampOffsetS;
};

using Trajectory = std::vector<TrajectoryPoint>;

class ITrajectoryGenerator {
public:
    virtual ~ITrajectoryGenerator() = default;
    virtual Trajectory generate(const Pose& ego,
                                const std::vector<TrajectoryPoint>& waypoints) = 0;
};

// Quintic polynomial spline trajectory generator.
class TrajectoryGenerator : public ITrajectoryGenerator {
public:
    Trajectory generate(const Pose& ego,
                        const std::vector<TrajectoryPoint>& waypoints) override;
};

} // namespace sdv::hpc::ad
