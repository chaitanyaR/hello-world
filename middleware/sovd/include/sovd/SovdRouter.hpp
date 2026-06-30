#pragma once

#include "SovdResource.hpp"
#include "SovdAuthentication.hpp"
#include <memory>
#include <string>
#include <vector>

namespace sdv::sovd {

struct Route {
    std::string                  pathPrefix;
    std::shared_ptr<SovdResource> resource;
};

// URL pattern → resource dispatch.
// Routes are matched by longest-prefix; the remaining suffix is passed in
// HttpRequest::path so resources can handle sub-paths (e.g., /faults/{id}).
class SovdRouter {
public:
    explicit SovdRouter(SovdAuthentication& auth);

    void mount(std::string pathPrefix, std::shared_ptr<SovdResource> resource);

    HttpResponse dispatch(HttpRequest req) const;

private:
    SovdAuthentication&      auth_;
    std::vector<Route>       routes_;

    const Route* findRoute(const std::string& path) const;
};

} // namespace sdv::sovd
