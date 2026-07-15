#include <powertrain/EngineController.hpp>
#include <algorithm>

namespace sdv::zones::powertrain {

void EngineController::setTorqueDemandNm(float nm)
{
    torque_ = std::clamp(nm, 0.0f, 500.0f);
}

float EngineController::currentTorqueNm() const { return torque_; }

} // namespace sdv::zones::powertrain
