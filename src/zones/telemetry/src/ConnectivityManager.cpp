#include <telemetry/ConnectivityManager.hpp>

namespace sdv::zones::telemetry {

bool ConnectivityManager::isLteConnected() const
{
    return signal_ > 0;
}

uint8_t ConnectivityManager::signalStrengthPercent() const
{
    return signal_;
}

void ConnectivityManager::setSignalStrength(uint8_t percent)
{
    signal_ = percent;
}

} // namespace sdv::zones::telemetry
