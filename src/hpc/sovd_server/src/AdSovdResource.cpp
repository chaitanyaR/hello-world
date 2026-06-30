#include <sovd_server/AdSovdResource.hpp>
#include <nlohmann/json.hpp>
namespace sdv::hpc::sovd_server {
sdv::sovd::HttpResponse AdSovdResource::handleGet(const sdv::sovd::HttpRequest&)
{
    nlohmann::json j; j["adStackState"] = "lane_keep"; return ok(j.dump());
}
}
