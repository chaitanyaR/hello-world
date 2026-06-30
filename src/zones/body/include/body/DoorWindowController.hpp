#pragma once
namespace sdv::zones::body {
enum class DoorId { FrontLeft, FrontRight, RearLeft, RearRight, Trunk };
class DoorWindowController {
public:
    void setLock(DoorId door, bool locked);
    bool isLocked(DoorId door) const;
};
}
