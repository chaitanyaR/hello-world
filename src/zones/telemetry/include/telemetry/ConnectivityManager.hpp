#pragma once
#include <cstdint>
namespace sdv::zones::telemetry {
class ConnectivityManager {
public:
    bool isLteConnected() const;
    uint8_t signalStrengthPercent() const;
private:
    uint8_t signal_{0};
};
}
