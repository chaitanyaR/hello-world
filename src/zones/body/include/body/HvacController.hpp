#pragma once
#include <cstdint>

namespace sdv::zones::body {

struct HvacCommand {
    float   driverSetpointCelsius;
    float   passengerSetpointCelsius;
    bool    acEnabled;
    uint8_t fanSpeedPercent;
};

struct HvacState {
    float   driverActualCelsius;
    float   passengerActualCelsius;
    bool    acRunning;
    uint8_t fanSpeedPercent;
};

class HvacController {
public:
    void apply(const HvacCommand& cmd);
    HvacState currentState() const;
private:
    HvacState state_{};
};

} // namespace sdv::zones::body
