#include "sovd/SovdResource.hpp"
#include <nlohmann/json.hpp>

namespace sdv::sovd {

HttpResponse SovdResource::handleGet(const HttpRequest&)
{
    return error(405, "Method Not Allowed", "GET not supported on this resource");
}

HttpResponse SovdResource::handlePost(const HttpRequest&)
{
    return error(405, "Method Not Allowed", "POST not supported on this resource");
}

HttpResponse SovdResource::handleDelete(const HttpRequest&)
{
    return error(405, "Method Not Allowed", "DELETE not supported on this resource");
}

HttpResponse SovdResource::ok(const std::string& jsonBody)
{
    return {200, jsonBody};
}

HttpResponse SovdResource::accepted(const std::string& location)
{
    nlohmann::json j;
    j["location"] = location;
    return {202, j.dump()};
}

HttpResponse SovdResource::noContent()
{
    return {204, ""};
}

HttpResponse SovdResource::error(int code, const std::string& msg,
                                  const std::string& detail)
{
    nlohmann::json j;
    j["code"]    = code;
    j["message"] = msg;
    if (!detail.empty()) j["detail"] = detail;
    return {code, j.dump()};
}

} // namespace sdv::sovd
