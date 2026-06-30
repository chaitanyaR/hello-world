#include <sovd_server/ZoneForwarder.hpp>
#include <nlohmann/json.hpp>
namespace sdv::hpc::sovd_server {
ZoneForwarder::ZoneForwarder(std::string zoneName,
                              sdv::someip::ServiceId serviceId,
                              sdv::someip::InstanceId instanceId,
                              sdv::someip::ServiceRegistry& registry)
    : zoneName_(std::move(zoneName)), serviceId_(serviceId),
      instanceId_(instanceId), registry_(registry)
{}
sdv::sovd::HttpResponse ZoneForwarder::handleGet(const sdv::sovd::HttpRequest&)
{
    nlohmann::json j;
    j["zone"]   = zoneName_;
    j["status"] = "stub";
    return ok(j.dump());
}
sdv::sovd::HttpResponse ZoneForwarder::handlePost(const sdv::sovd::HttpRequest& req)
{
    return accepted("/api/sovd/v1/zones/" + zoneName_ + "/status");
}
sdv::sovd::HttpResponse ZoneForwarder::handleDelete(const sdv::sovd::HttpRequest&)
{
    return noContent();
}
}
