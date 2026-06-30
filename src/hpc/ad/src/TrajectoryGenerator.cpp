#include <ad/TrajectoryGenerator.hpp>
#include <cmath>
#include <array>

namespace sdv::hpc::ad {

// Quintic polynomial spline: s(t) = a0 + a1*t + a2*t^2 + a3*t^3 + a4*t^4 + a5*t^5
// Boundary conditions: s(0)=x0, s'(0)=v0, s''(0)=a0, s(T)=x1, s'(T)=v1, s''(T)=a1
// For simulation we use zero boundary velocities and accelerations.
static float quinticAt(float x0, float x1, float T, float t)
{
    if (T <= 0.0f) return x1;
    float tau = t / T;
    float t2 = tau * tau;
    float t3 = t2 * tau;
    float t4 = t3 * tau;
    float t5 = t4 * tau;
    // Coefficients for zero-velocity/acceleration boundaries:
    // a0=x0, a1=0, a2=0, a3=10(x1-x0), a4=-15(x1-x0), a5=6(x1-x0)
    float dx = x1 - x0;
    return x0 + dx * (10.0f*t3 - 15.0f*t4 + 6.0f*t5);
}

Trajectory TrajectoryGenerator::generate(const Pose& ego,
                                          const std::vector<TrajectoryPoint>& waypoints)
{
    if (waypoints.empty()) return {};

    Trajectory result;
    result.reserve(waypoints.size() * 10);

    float prevX = static_cast<float>(ego.longitudeDeg);
    float prevY = static_cast<float>(ego.latitudeDeg);
    float prevT = 0.0f;

    for (const auto& wp : waypoints) {
        float segDuration = wp.timestampOffsetS - prevT;
        if (segDuration <= 0.0f) { result.push_back(wp); prevT = wp.timestampOffsetS; continue; }

        int steps = 10;
        for (int i = 1; i <= steps; ++i) {
            float frac = static_cast<float>(i) / static_cast<float>(steps);
            float t    = segDuration * frac;
            TrajectoryPoint pt{};
            pt.x                = quinticAt(prevX, wp.x, segDuration, t);
            pt.y                = quinticAt(prevY, wp.y, segDuration, t);
            pt.speedMps         = wp.speedMps;
            pt.headingDeg       = wp.headingDeg;
            pt.curvature        = wp.curvature;
            pt.timestampOffsetS = prevT + t;
            result.push_back(pt);
        }
        prevX = wp.x;
        prevY = wp.y;
        prevT = wp.timestampOffsetS;
    }
    return result;
}

} // namespace sdv::hpc::ad
