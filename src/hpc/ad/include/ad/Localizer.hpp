#pragma once

#include <cstdint>

namespace sdv::hpc::ad {

struct Pose {
    double latitudeDeg;
    double longitudeDeg;
    float  altitudeM;
    float  headingDeg;
    float  speedMps;
    float  uncertaintyM;
    uint64_t timestampUs;
};

struct GnssMeasurement {
    double latitudeDeg;
    double longitudeDeg;
    float  altitudeM;
    float  hdop;
};

struct ImuMeasurement {
    float accelX, accelY, accelZ;   // m/s²
    float gyroX,  gyroY,  gyroZ;   // rad/s
    uint64_t timestampUs;
};

class ILocalizer {
public:
    virtual ~ILocalizer() = default;
    virtual void update(const GnssMeasurement& gnss, const ImuMeasurement& imu) = 0;
    virtual Pose currentPose() const = 0;
};

// Extended Kalman Filter fusing GNSS + IMU.
class Localizer : public ILocalizer {
public:
    void update(const GnssMeasurement& gnss, const ImuMeasurement& imu) override;
    Pose currentPose() const override;
private:
    Pose pose_{};
};

} // namespace sdv::hpc::ad
