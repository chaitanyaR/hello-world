#include <powertrain/TorqueArbiter.hpp>
#include <algorithm>

namespace sdv::zones::powertrain {

void TorqueArbiter::submit(TorqueRequest req)
{
    std::lock_guard lock(mutex_);
    queue_.push(req);
}

float TorqueArbiter::arbitrate(float maxCapabilityNm)
{
    std::lock_guard lock(mutex_);
    if (queue_.empty()) {
        return lastGranted_;
    }
    TorqueRequest top = queue_.top();
    // Drain queue
    while (!queue_.empty()) queue_.pop();

    float granted = std::min(top.requestedNm, maxCapabilityNm);
    granted = std::max(granted, 0.0f);
    lastGranted_ = granted;
    return granted;
}

float TorqueArbiter::lastGrantedNm() const
{
    std::lock_guard lock(mutex_);
    return lastGranted_;
}

} // namespace sdv::zones::powertrain
