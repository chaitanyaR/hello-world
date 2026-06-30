#pragma once
#include <cstdint>
namespace sdv::zones::body {
enum class LightingZone { Headlights, TailLights, InteriorFront, InteriorRear, Ambient };
enum class LightingState { Off, On, Dim50, Dim25 };
class LightingController {
public:
    void set(LightingZone zone, LightingState state);
};
}
