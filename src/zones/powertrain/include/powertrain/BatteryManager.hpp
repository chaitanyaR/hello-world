#pragma once

#include <cstdint>

namespace sdv::zones::powertrain {

struct BatteryState {
    float   socPercent;
    float   voltageV;
    float   currentA;
    float   tempCelsius;
    bool    isCharging;
};

enum class BatteryFsmState { Idle, Charging, Discharging, Fault };

// BEV/HEV battery state machine: IDLE / CHARGING / DISCHARGING / FAULT.
// Transitions driven by charger contact, load demand, and temperature.
class BatteryManager {
public:
    void update(float chargerVoltageV, float loadCurrentA, float tempCelsius);

    BatteryState   currentState() const;
    BatteryFsmState fsmState() const;

private:
    BatteryState    state_{};
    BatteryFsmState fsm_{BatteryFsmState::Idle};

    void transitionTo(BatteryFsmState next);
};

} // namespace sdv::zones::powertrain
