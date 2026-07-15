#pragma once
#include <array>
namespace sdv::zones::body {
class ComfortManager {
public:
    static constexpr int kMaxSeats = 4;
    void  setSeatPosition(int seatId, float positionPercent);
    float seatPosition(int seatId) const;
private:
    std::array<float, kMaxSeats> seatPositions_{};
};
}
