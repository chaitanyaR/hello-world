#pragma once
#include <cstdint>
namespace sdv::zones::telemetry {
class ConnectivityManager {
public:
    bool    isLteConnected() const;
    uint8_t signalStrengthPercent() const;
    void    setSignalStrength(uint8_t percent);
private:
    uint8_t signal_{0};
};
}
