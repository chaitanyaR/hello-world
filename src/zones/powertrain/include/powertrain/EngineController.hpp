#pragma once
namespace sdv::zones::powertrain {
class EngineController {
public:
    void setTorqueDemandNm(float nm);
    float currentTorqueNm() const;
private:
    float torque_{0.0f};
};
}
