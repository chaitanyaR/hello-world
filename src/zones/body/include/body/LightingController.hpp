#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace sdv::zones::body {
enum class LightingZone  { Headlights, TailLights, InteriorFront, InteriorRear, Ambient };
enum class LightingState { Off, On, Dim50, Dim25 };
class LightingController {
public:
    static constexpr std::size_t kMaxZones = 5;
    void          set(LightingZone zone, LightingState state);
    LightingState get(LightingZone zone) const;
    void          allOff();
private:
    std::array<LightingState, kMaxZones> states_{};
};
}
