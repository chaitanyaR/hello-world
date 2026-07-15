#pragma once

#include <cstdint>

namespace sdv::zones::adas_zone {

struct ActuatorCommand {
    float   steeringAngleDeg;
    float   throttlePercent;
    float   brakePressureBar;
    bool    emergencyStop{false};
    uint64_t timestampUs;
};

class ActuatorController {
public:
    // Send the command to the physical (or simulated) actuator hardware.
    void apply(const ActuatorCommand& cmd);
    const ActuatorCommand& lastApplied() const;
private:
    ActuatorCommand last_{};
};

} // namespace sdv::zones::adas_zone
