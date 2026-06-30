#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

namespace sdv::hpc::adas {

using SafeStateCallback = std::function<void()>;

// ASIL-D independent watchdog.
// AdasPipeline calls heartbeat() every tick. If heartbeat() is not called
// within the deadline, SafetyMonitor fires the safe-state callback and sets
// the system into SAFE_STOP via the SOME/IP event.
class ISafetyMonitor {
public:
    virtual ~ISafetyMonitor() = default;
    virtual void heartbeat() = 0;
    virtual bool isHealthy() const = 0;
};

class SafetyMonitor : public ISafetyMonitor {
public:
    explicit SafetyMonitor(std::chrono::milliseconds deadline,
                           SafeStateCallback onSafeState);
    ~SafetyMonitor();

    void heartbeat() override;
    bool isHealthy() const override;

private:
    void watchdogLoop();

    std::chrono::milliseconds         deadline_;
    SafeStateCallback                 onSafeState_;
    std::atomic<bool>                 healthy_{true};
    std::atomic<bool>                 running_{true};
    std::chrono::steady_clock::time_point lastHeartbeat_;
    mutable std::mutex                mutex_;
    std::thread                       watchdog_;
};

} // namespace sdv::hpc::adas
