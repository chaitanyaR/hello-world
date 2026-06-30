#include <powertrain/BatteryManager.hpp>
#include <algorithm>

namespace sdv::zones::powertrain {

void BatteryManager::update(float chargerVoltageV, float loadCurrentA, float tempCelsius)
{
    state_.voltageV    = chargerVoltageV > 0.0f ? chargerVoltageV : 380.0f;
    state_.currentA    = loadCurrentA;
    state_.tempCelsius = tempCelsius;

    if (tempCelsius > 55.0f || loadCurrentA > 300.0f) {
        transitionTo(BatteryFsmState::Fault);
        return;
    }

    switch (fsm_) {
    case BatteryFsmState::Idle:
        if (chargerVoltageV > 100.0f)  transitionTo(BatteryFsmState::Charging);
        else if (loadCurrentA > 5.0f)  transitionTo(BatteryFsmState::Discharging);
        break;
    case BatteryFsmState::Charging:
        if (state_.socPercent >= 100.0f || chargerVoltageV < 10.0f)
            transitionTo(BatteryFsmState::Idle);
        else
            state_.socPercent = std::min(100.0f, state_.socPercent + 0.5f);
        break;
    case BatteryFsmState::Discharging:
        if (loadCurrentA < 1.0f)
            transitionTo(BatteryFsmState::Idle);
        else
            state_.socPercent = std::max(0.0f, state_.socPercent - 0.1f);
        break;
    case BatteryFsmState::Fault:
        break; // latched — requires external reset
    }
}

BatteryState    BatteryManager::currentState() const { return state_; }
BatteryFsmState BatteryManager::fsmState()     const { return fsm_; }

void BatteryManager::transitionTo(BatteryFsmState next)
{
    fsm_ = next;
    state_.isCharging = (next == BatteryFsmState::Charging);
}

} // namespace sdv::zones::powertrain
