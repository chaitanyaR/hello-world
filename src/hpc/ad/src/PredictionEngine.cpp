#include <ad/PredictionEngine.hpp>
namespace sdv::hpc::ad {
std::vector<AgentPrediction> PredictionEngine::predict(const std::vector<Agent>& agents,
                                                        float horizonS)
{
    std::vector<AgentPrediction> preds;
    const float dt = 0.1f;
    const int steps = static_cast<int>(horizonS / dt);
    for (const auto& a : agents) {
        AgentPrediction p;
        p.agentId = a.id;
        for (int i = 1; i <= steps; ++i)
            p.futurePositions.push_back({a.x + a.vx * i * dt,
                                         a.y + a.vy * i * dt});
        preds.push_back(std::move(p));
    }
    return preds;
}
}
