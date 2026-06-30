#include "sovd/SovdDataAccessor.hpp"
#include <nlohmann/json.hpp>

namespace sdv::sovd {

SovdDataAccessor::SovdDataAccessor(std::string name, DataReader reader)
    : name_(std::move(name)), reader_(std::move(reader))
{}

SovdDataAccessor::SovdDataAccessor(std::string name, DataReader reader,
                                   DataWriter writer)
    : name_(std::move(name)), reader_(std::move(reader)),
      writer_(std::move(writer))
{}

HttpResponse SovdDataAccessor::handleGet(const HttpRequest&)
{
    DataElement elem = reader_();
    nlohmann::json j;
    j["name"]        = elem.name;
    j["value"]       = elem.value;
    j["unit"]        = elem.unit;
    j["timestampUs"] = elem.timestampUs;
    return ok(j.dump());
}

HttpResponse SovdDataAccessor::handlePost(const HttpRequest& req)
{
    if (!writer_) {
        return error(405, "Method Not Allowed", "Data element is read-only");
    }
    if (!writer_(req.body)) {
        return error(422, "Unprocessable Entity", "Write rejected by ECU");
    }
    return ok("{}");
}

} // namespace sdv::sovd
