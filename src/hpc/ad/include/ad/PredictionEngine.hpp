#pragma once

#include "Localizer.hpp"
#include <cstdint>
#include <vector>

namespace sdv::hpc::ad {

struct Agent {
    uint32_t id;
    float    x, y;
    float    vx, vy;
    float    headingDeg;
};

struct AgentPrediction {
    uint32_t          agentId;
    std::vector<std::pair<float,float>> futurePositions;  // (x,y) per 0.1s step
};

class IPredictionEngine {
public:
    virtual ~IPredictionEngine() = default;
    virtual std::vector<AgentPrediction> predict(const std::vector<Agent>& agents,
                                                  float horizonS) = 0;
};

// Constant-velocity prediction model.
class PredictionEngine : public IPredictionEngine {
public:
    std::vector<AgentPrediction> predict(const std::vector<Agent>& agents,
                                          float horizonS) override;
};

} // namespace sdv::hpc::ad
