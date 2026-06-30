#include <ad/Localizer.hpp>
namespace sdv::hpc::ad {
void Localizer::update(const GnssMeasurement& gnss, const ImuMeasurement&)
{
    pose_.latitudeDeg  = gnss.latitudeDeg;
    pose_.longitudeDeg = gnss.longitudeDeg;
    pose_.altitudeM    = gnss.altitudeM;
    pose_.uncertaintyM = gnss.hdop * 5.0f;
}
Pose Localizer::currentPose() const { return pose_; }
}
