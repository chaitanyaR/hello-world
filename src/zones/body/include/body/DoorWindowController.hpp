#pragma once
#include <array>
#include <cstddef>
namespace sdv::zones::body {
enum class DoorId { FrontLeft, FrontRight, RearLeft, RearRight, Trunk };
class DoorWindowController {
public:
    static constexpr std::size_t kMaxDoors = 5;
    void setLock(DoorId door, bool locked);
    bool isLocked(DoorId door) const;
private:
    std::array<bool, kMaxDoors> locked_{};
};
}
