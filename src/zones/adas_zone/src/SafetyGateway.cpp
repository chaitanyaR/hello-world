#include <adas_zone/SafetyGateway.hpp>
#include <chrono>
#include <cmath>

namespace sdv::zones::adas_zone {

using Clock = std::chrono::steady_clock;

SafetyGateway::SafetyGateway(ActuatorController& controller, SafetyLimits limits)
    : controller_(controller), limits_(limits),
      lastCmdTime_(Clock::now() - std::chrono::seconds(1))
{}

bool SafetyGateway::applyCommand(const ActuatorCommand& cmd)
{
    if (!checkPlausibility(cmd) || !checkRateLimits(cmd)) {
        violation_ = true;
        controller_.apply(lastValid_);
        return false;
    }
    lastValid_   = cmd;
    lastCmdTime_ = Clock::now();
    controller_.apply(cmd);
    return true;
}

bool SafetyGateway::isViolation() const { return violation_; }

bool SafetyGateway::checkPlausibility(const ActuatorCommand& cmd) const
{
    if (std::fabs(cmd.steeringAngleDeg) > limits_.maxSteeringAngleDeg)  return false;
    if (cmd.brakePressureBar           > limits_.maxBrakePressureBar)   return false;
    if (cmd.throttlePercent < 0.0f || cmd.throttlePercent > 100.0f)     return false;
    return true;
}

bool SafetyGateway::checkRateLimits(const ActuatorCommand& cmd) const
{
    auto now     = Clock::now();
    float dtS    = std::chrono::duration<float>(now - lastCmdTime_).count();
    if (dtS <= 0.0f) return true;

    float steeringDelta = std::fabs(cmd.steeringAngleDeg - lastValid_.steeringAngleDeg);
    if (steeringDelta / dtS > limits_.maxSteeringRateDegPerS) return false;

    float throttleDelta = std::fabs(cmd.throttlePercent - lastValid_.throttlePercent);
    if (throttleDelta / dtS > limits_.maxThrottleRatePercentPerS) return false;

    return true;
}

} // namespace sdv::zones::adas_zone
