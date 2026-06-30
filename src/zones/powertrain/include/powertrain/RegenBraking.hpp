#pragma once
namespace sdv::zones::powertrain {
enum class RegenMode { Off, Low, High };
class RegenBraking {
public:
    void setMode(RegenMode mode);
    float recoveredTorqueNm(float vehicleSpeedMps) const;
private:
    RegenMode mode_{RegenMode::Off};
};
}
