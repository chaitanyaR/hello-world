#include <ad/Localizer.hpp>
#include <cmath>

namespace sdv::hpc::ad {

// 2-state EKF: position from GNSS, velocity integrated from IMU.
// State vector: [lat, lon, speedMps, headingDeg]
// Simplified scalar update — sufficient for simulation fidelity.

static constexpr float kDt = 0.02f; // 50 Hz prediction step

void Localizer::update(const GnssMeasurement& gnss, const ImuMeasurement& imu)
{
    // IMU integration: accumulate velocity and heading
    constexpr float kG = 9.81f;
    float ax = imu.accelX;
    float ay = imu.accelY;
    float accelForward = std::sqrt(ax * ax + ay * ay) * kDt;

    velMs_    += accelForward;
    headingDeg_ += imu.gyroZ * (180.0f / 3.14159265f) * kDt;

    // Kalman gain blends GNSS position with IMU-integrated position
    // For simulation use fixed gain K=0.8 (favours GNSS).
    constexpr float K = 0.8f;
    pose_.latitudeDeg  = K * gnss.latitudeDeg  + (1.0f - K) * pose_.latitudeDeg;
    pose_.longitudeDeg = K * gnss.longitudeDeg + (1.0f - K) * pose_.longitudeDeg;
    pose_.altitudeM    = gnss.altitudeM;
    pose_.uncertaintyM = gnss.hdop * 3.0f;

    // Integrate position from velocity when GNSS uncertainty is high
    if (gnss.hdop > 5.0f) {
        float headRad = headingDeg_ * 3.14159265f / 180.0f;
        pose_.latitudeDeg  += velMs_ * std::cos(headRad) * kDt * 9e-6f;
        pose_.longitudeDeg += velMs_ * std::sin(headRad) * kDt * 9e-6f;
    }

    pose_.speedMps  = velMs_;
    pose_.headingDeg = headingDeg_;
    pose_.timestampUs = imu.timestampUs;
}

Pose Localizer::currentPose() const { return pose_; }

} // namespace sdv::hpc::ad
