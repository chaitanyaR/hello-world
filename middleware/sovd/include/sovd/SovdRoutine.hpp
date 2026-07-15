#pragma once

#include "SovdResource.hpp"
#include <functional>
#include <mutex>
#include <thread>

namespace sdv::sovd {

using RoutineExecutor = std::function<RoutineStatus()>;

// SOVD routine resource: POST starts the routine asynchronously.
// GET returns the current RoutineStatus.
class SovdRoutine : public SovdResource {
public:
    explicit SovdRoutine(std::string routineId, RoutineExecutor executor);
    ~SovdRoutine();

    // POST /routines/{id}        → 202 Accepted
    // GET  /routines/{id}/status → 200 + RoutineStatus JSON
    HttpResponse handlePost(const HttpRequest& req) override;
    HttpResponse handleGet(const HttpRequest& req) override;

private:
    std::string      routineId_;
    RoutineExecutor  executor_;
    mutable std::mutex mutex_;
    RoutineStatus    status_;
    std::thread      worker_;

    void runAsync();
};

} // namespace sdv::sovd
