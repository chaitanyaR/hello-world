#include "sovd/SovdRouter.hpp"
#include <algorithm>

namespace sdv::sovd {

SovdRouter::SovdRouter(SovdAuthentication& auth) : auth_(auth) {}

void SovdRouter::mount(std::string pathPrefix,
                       std::shared_ptr<SovdResource> resource)
{
    routes_.push_back({std::move(pathPrefix), std::move(resource)});
    // Keep longest-prefix first for correct matching
    std::sort(routes_.begin(), routes_.end(),
              [](const Route& a, const Route& b) {
                  return a.pathPrefix.size() > b.pathPrefix.size();
              });
}

HttpResponse SovdRouter::dispatch(HttpRequest req) const
{
    // Authenticate
    const auto authHeader = req.query.count("Authorization")
                                ? req.query.at("Authorization")
                                : std::string{};
    req.callerRole = auth_.authenticate(authHeader);

    auto required = SovdAuthentication::minimumRoleForPath(req.path);
    if (static_cast<int>(req.callerRole) < static_cast<int>(required)) {
        return {401, R"({"code":401,"message":"Unauthorized"})"};
    }

    const Route* route = findRoute(req.path);
    if (!route) {
        return {404, R"({"code":404,"message":"Resource not found"})"};
    }

    if (req.method == "GET")    return route->resource->handleGet(req);
    if (req.method == "POST")   return route->resource->handlePost(req);
    if (req.method == "DELETE") return route->resource->handleDelete(req);
    return {405, R"({"code":405,"message":"Method Not Allowed"})"};
}

const Route* SovdRouter::findRoute(const std::string& path) const
{
    for (const auto& route : routes_) {
        if (path.starts_with(route.pathPrefix)) {
            return &route;
        }
    }
    return nullptr;
}

} // namespace sdv::sovd
