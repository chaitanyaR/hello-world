#include <sovd_server/AdasSovdResource.hpp>
#include <nlohmann/json.hpp>
namespace sdv::hpc::sovd_server {
sdv::sovd::HttpResponse AdasSovdResource::handleGet(const sdv::sovd::HttpRequest&)
{
    nlohmann::json j; j["pipelineState"] = "nominal"; return ok(j.dump());
}
}
