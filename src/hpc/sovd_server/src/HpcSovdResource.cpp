#include <sovd_server/HpcSovdResource.hpp>
#include <nlohmann/json.hpp>
namespace sdv::hpc::sovd_server {
HpcSovdResource::HpcSovdResource() = default;
void HpcSovdResource::reportFault(sdv::sovd::DtcEntry entry)
{
    faultMemory_.add(std::move(entry));
}
sdv::sovd::HttpResponse HpcSovdResource::handleGet(const sdv::sovd::HttpRequest& req)
{
    nlohmann::json j;
    j["node"]  = "hpc";
    j["state"] = "nominal";
    return ok(j.dump());
}
}
