#include <body/HvacController.hpp>

namespace sdv::zones::body {

void HvacController::apply(const HvacCommand& cmd)
{
    state_.driverActualCelsius    = cmd.driverSetpointCelsius;
    state_.passengerActualCelsius = cmd.passengerSetpointCelsius;
    state_.acRunning              = cmd.acEnabled;
    state_.fanSpeedPercent        = cmd.fanSpeedPercent;
}

HvacState HvacController::currentState() const { return state_; }

} // namespace sdv::zones::body
