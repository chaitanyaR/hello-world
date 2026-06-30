#pragma once

#include "ActuatorController.hpp"
#include <chrono>

namespace sdv::zones::adas_zone {

struct SafetyLimits {
    float maxSteeringRateDegPerS     = 200.0f;
    float maxThrottleRatePercentPerS = 50.0f;
    float maxBrakePressureBar        = 150.0f;
    float maxSteeringAngleDeg        = 540.0f;
};

// Safety Gateway decorator around ActuatorController.
// Enforces rate limits, plausibility bounds, and hold-last-valid logic.
// Any violation triggers an immediate safe-state request to HPC.
class SafetyGateway {
public:
    explicit SafetyGateway(ActuatorController& controller,
                           SafetyLimits limits = SafetyLimits{});

    // Returns true if the command passed all checks and was sent to controller.
    // Returns false and holds last valid on any violation.
    bool applyCommand(const ActuatorCommand& cmd);

    bool isViolation() const;

private:
    ActuatorController& controller_;
    SafetyLimits        limits_;
    ActuatorCommand     lastValid_{};
    bool                violation_{false};
    std::chrono::steady_clock::time_point lastCmdTime_;

    bool checkPlausibility(const ActuatorCommand& cmd) const;
    bool checkRateLimits(const ActuatorCommand& cmd) const;
};

} // namespace sdv::zones::adas_zone
