#pragma once

#include "SovdTypes.hpp"
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace sdv::sovd {

using QueryParams = std::unordered_map<std::string, std::string>;

struct HttpRequest {
    std::string  method;   // "GET" | "POST" | "DELETE"
    std::string  path;
    std::string  body;     // JSON string
    QueryParams  query;
    SovdRole     callerRole{SovdRole::Unauthenticated};
};

struct HttpResponse {
    int         statusCode{200};
    std::string body;      // JSON string
};

// Abstract base for all SOVD REST resources.
// Each concrete resource (data element, fault memory, routine) derives from
// this and overrides the HTTP verbs it supports.
class SovdResource {
public:
    virtual ~SovdResource() = default;

    virtual HttpResponse handleGet(const HttpRequest& req);
    virtual HttpResponse handlePost(const HttpRequest& req);
    virtual HttpResponse handleDelete(const HttpRequest& req);

protected:
    static HttpResponse ok(const std::string& jsonBody);
    static HttpResponse accepted(const std::string& location);
    static HttpResponse noContent();
    static HttpResponse error(int code, const std::string& msg,
                              const std::string& detail = {});
};

} // namespace sdv::sovd
