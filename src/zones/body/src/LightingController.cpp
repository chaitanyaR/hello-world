#include <body/LightingController.hpp>

namespace sdv::zones::body {

void LightingController::set(LightingZone zone, LightingState state)
{
    auto idx = static_cast<std::size_t>(zone);
    if (idx < kMaxZones) {
        states_[idx] = state;
    }
}

LightingState LightingController::get(LightingZone zone) const
{
    auto idx = static_cast<std::size_t>(zone);
    if (idx < kMaxZones) return states_[idx];
    return LightingState::Off;
}

void LightingController::allOff()
{
    for (auto& s : states_) s = LightingState::Off;
}

} // namespace sdv::zones::body
