#pragma once

#include "PathPlanner.hpp"
#include "ObjectTracker.hpp"
#include <functional>

namespace sdv::hpc::adas {

struct ActuatorCommand {
    float   steeringAngleDeg;
    float   throttlePercent;
    float   brakePressureBar;
    bool    emergencyStop{false};
    uint64_t timestampUs;
};

enum class DrivingState { Nominal, Hazard, EmergencyStop };

using ActuatorOutputCallback = std::function<void(const ActuatorCommand&)>;

class IDecisionMaker {
public:
    virtual ~IDecisionMaker() = default;
    virtual ActuatorCommand decide(const Path& path, const TrackList& tracks,
                                   const EgoState& ego) = 0;
    virtual DrivingState    currentState() const = 0;
};

// Finite state machine: NOMINAL / HAZARD / EMERGENCY_STOP.
// Outputs ActuatorCommand; calls registered callback for SOME/IP dispatch.
class DecisionMaker : public IDecisionMaker {
public:
    void setOutputCallback(ActuatorOutputCallback cb);

    ActuatorCommand decide(const Path& path, const TrackList& tracks,
                           const EgoState& ego) override;
    DrivingState currentState() const override;

private:
    DrivingState           state_{DrivingState::Nominal};
    ActuatorOutputCallback outputCb_;

    bool hazardDetected(const TrackList& tracks, const EgoState& ego) const;
};

} // namespace sdv::hpc::adas
