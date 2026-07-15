#include "sovd/SovdRoutine.hpp"
#include <nlohmann/json.hpp>

namespace sdv::sovd {

SovdRoutine::SovdRoutine(std::string routineId, RoutineExecutor executor)
    : routineId_(std::move(routineId)), executor_(std::move(executor))
{
    status_.routineId = routineId_;
}

SovdRoutine::~SovdRoutine()
{
    if (worker_.joinable()) worker_.join();
}

HttpResponse SovdRoutine::handlePost(const HttpRequest& req)
{
    {
        std::lock_guard lock(mutex_);
        if (status_.state == RoutineState::Running ||
            status_.state == RoutineState::Pending) {
            return error(409, "Conflict", "Routine already in progress");
        }
        status_.state           = RoutineState::Pending;
        status_.progressPercent = 0;
        status_.resultMessage   = {};
    }

    if (worker_.joinable()) worker_.join();
    worker_ = std::thread([this] { runAsync(); });

    return accepted("/api/sovd/v1/routines/" + routineId_ + "/status");
}

HttpResponse SovdRoutine::handleGet(const HttpRequest&)
{
    std::lock_guard lock(mutex_);
    nlohmann::json j;
    j["routineId"]       = status_.routineId;
    j["state"]           = static_cast<int>(status_.state);
    j["progressPercent"] = status_.progressPercent;
    j["resultMessage"]   = status_.resultMessage;
    return ok(j.dump());
}

void SovdRoutine::runAsync()
{
    {
        std::lock_guard lock(mutex_);
        status_.state = RoutineState::Running;
    }
    RoutineStatus result = executor_();
    std::lock_guard lock(mutex_);
    status_ = std::move(result);
}

} // namespace sdv::sovd
