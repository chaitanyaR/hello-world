#include <body/DoorWindowController.hpp>

namespace sdv::zones::body {

static constexpr std::size_t doorIndex(DoorId d)
{
    return static_cast<std::size_t>(d);
}

void DoorWindowController::setLock(DoorId door, bool locked)
{
    std::size_t i = doorIndex(door);
    if (i < kMaxDoors) locked_[i] = locked;
}

bool DoorWindowController::isLocked(DoorId door) const
{
    std::size_t i = doorIndex(door);
    return i < kMaxDoors && locked_[i];
}

} // namespace sdv::zones::body
