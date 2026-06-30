#pragma once
#include <cstdint>
#include <vector>
namespace sdv::zones::adas_zone {
struct SensorFrame { uint64_t timestampUs; };
class SensorAggregator {
public:
    SensorFrame aggregate();
};
}
