#pragma once

#include <cstdint>
#include <queue>
#include <mutex>

namespace sdv::zones::powertrain {

struct TorqueRequest {
    float    requestedNm;
    uint8_t  priority;    // 0 = highest (safety), 7 = lowest
    uint64_t timestampUs;
};

// Priority-queue torque arbitration with rate limiting and hold-last-valid.
// Higher-priority requests (lower priority value) always win.
class TorqueArbiter {
public:
    // Submit a torque request from HPC or local control.
    void submit(TorqueRequest req);

    // Returns the highest-priority granted torque for this tick.
    float arbitrate(float maxCapabilityNm);

    float lastGrantedNm() const;

private:
    struct Compare {
        bool operator()(const TorqueRequest& a, const TorqueRequest& b) const {
            return a.priority > b.priority; // min-heap on priority value
        }
    };

    mutable std::mutex mutex_;
    std::priority_queue<TorqueRequest,
                        std::vector<TorqueRequest>,
                        Compare>   queue_;
    float lastGranted_{0.0f};
};

} // namespace sdv::zones::powertrain
