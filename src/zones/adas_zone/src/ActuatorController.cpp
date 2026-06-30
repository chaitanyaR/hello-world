#include <adas_zone/ActuatorController.hpp>

namespace sdv::zones::adas_zone {

void ActuatorController::apply(const ActuatorCommand& cmd)
{
    last_ = cmd;
}

const ActuatorCommand& ActuatorController::lastApplied() const
{
    return last_;
}

} // namespace sdv::zones::adas_zone
