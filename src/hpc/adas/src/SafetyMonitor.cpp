#include <adas/SafetyMonitor.hpp>
#include <mutex>

namespace sdv::hpc::adas {

SafetyMonitor::SafetyMonitor(std::chrono::milliseconds deadline,
                             SafeStateCallback onSafeState)
    : deadline_(deadline), onSafeState_(std::move(onSafeState))
{
    lastHeartbeat_ = std::chrono::steady_clock::now();
    watchdog_      = std::thread([this] { watchdogLoop(); });
}

SafetyMonitor::~SafetyMonitor()
{
    running_ = false;
    if (watchdog_.joinable()) watchdog_.join();
}

void SafetyMonitor::heartbeat()
{
    std::lock_guard lock(mutex_);
    lastHeartbeat_ = std::chrono::steady_clock::now();
    healthy_       = true;
}

bool SafetyMonitor::isHealthy() const { return healthy_.load(); }

void SafetyMonitor::watchdogLoop()
{
    while (running_.load()) {
        std::this_thread::sleep_for(deadline_ / 4);
        std::lock_guard lock(mutex_);
        auto now     = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastHeartbeat_);
        if (elapsed > deadline_ && healthy_.load()) {
            healthy_ = false;
            if (onSafeState_) onSafeState_();
        }
    }
}

} // namespace sdv::hpc::adas
