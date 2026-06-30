#include <adas/DecisionMaker.hpp>
#include <chrono>

namespace sdv::hpc::adas {

void DecisionMaker::setOutputCallback(ActuatorOutputCallback cb)
{
    outputCb_ = std::move(cb);
}

DrivingState DecisionMaker::currentState() const { return state_; }

ActuatorCommand DecisionMaker::decide(const Path& path, const TrackList& tracks,
                                      const EgoState& ego)
{
    ActuatorCommand cmd{};
    cmd.timestampUs = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());

    if (hazardDetected(tracks, ego)) {
        state_             = DrivingState::Hazard;
        cmd.brakePressureBar = 80.0f;
        cmd.throttlePercent  = 0.0f;
    } else {
        state_              = DrivingState::Nominal;
        cmd.throttlePercent = 30.0f;
        cmd.brakePressureBar= 0.0f;
    }

    if (outputCb_) outputCb_(cmd);
    return cmd;
}

bool DecisionMaker::hazardDetected(const TrackList& tracks,
                                    const EgoState& ego) const
{
    for (const auto& t : tracks) {
        float dx = t.x - ego.x;
        float dy = t.y - ego.y;
        float dist = dx * dx + dy * dy;
        if (dist < 100.0f) return true; // <10 m
    }
    return false;
}

} // namespace sdv::hpc::adas
