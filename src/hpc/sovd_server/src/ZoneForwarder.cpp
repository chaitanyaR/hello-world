#include <sovd_server/ZoneForwarder.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace sdv::hpc::sovd_server {

ZoneForwarder::ZoneForwarder(std::string zoneName,
                              sdv::someip::ServiceId serviceId,
                              sdv::someip::InstanceId instanceId,
                              sdv::someip::ServiceRegistry& registry)
    : zoneName_(std::move(zoneName)), serviceId_(serviceId),
      instanceId_(instanceId), registry_(registry)
{}

sdv::sovd::HttpResponse ZoneForwarder::handleGet(const sdv::sovd::HttpRequest& req)
{
    // Serve faults snapshot if path ends with /faults or /faults/*
    if (req.path.find("/faults") != std::string::npos) {
        return faultMemory_.handleGet(req);
    }
    nlohmann::json j;
    j["zone"]   = zoneName_;
    j["status"] = "online";
    j["faultCount"] = faultMemory_.snapshot().size();
    return ok(j.dump());
}

sdv::sovd::HttpResponse ZoneForwarder::handlePost(const sdv::sovd::HttpRequest& req)
{
    // Routine trigger — return 202 with polling location
    std::string location = "/api/sovd/v1/zones/" + zoneName_ + "/routines/status";
    return accepted(location);
}

sdv::sovd::HttpResponse ZoneForwarder::handleDelete(const sdv::sovd::HttpRequest& req)
{
    // Clear all faults for this zone
    if (req.path.find("/faults") != std::string::npos) {
        faultMemory_.clearAll();
    }
    return noContent();
}

void ZoneForwarder::injectFault(sdv::sovd::DtcEntry entry)
{
    faultMemory_.add(std::move(entry));
}

} // namespace sdv::hpc::sovd_server
