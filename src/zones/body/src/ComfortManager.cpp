#include <body/ComfortManager.hpp>
#include <algorithm>

namespace sdv::zones::body {

void ComfortManager::setSeatPosition(int seatId, float positionPercent)
{
    if (seatId >= 0 && seatId < kMaxSeats) {
        seatPositions_[seatId] = std::clamp(positionPercent, 0.0f, 100.0f);
    }
}

float ComfortManager::seatPosition(int seatId) const
{
    if (seatId >= 0 && seatId < kMaxSeats) return seatPositions_[seatId];
    return 0.0f;
}

} // namespace sdv::zones::body
