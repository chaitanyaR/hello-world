#include <powertrain/RegenBraking.hpp>

namespace sdv::zones::powertrain {

void RegenBraking::setMode(RegenMode mode) { mode_ = mode; }

float RegenBraking::recoveredTorqueNm(float vehicleSpeedMps) const
{
    switch (mode_) {
    case RegenMode::High:
        return vehicleSpeedMps * 8.0f;   // aggressive: ~8 Nm per m/s
    case RegenMode::Low:
        return vehicleSpeedMps * 3.0f;   // light: ~3 Nm per m/s
    case RegenMode::Off:
    default:
        return 0.0f;
    }
}

} // namespace sdv::zones::powertrain
