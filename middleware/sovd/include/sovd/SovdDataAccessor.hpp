#pragma once

#include "SovdResource.hpp"
#include <functional>
#include <string>

namespace sdv::sovd {

using DataReader = std::function<DataElement()>;
using DataWriter = std::function<bool(const std::string& jsonValue)>;

// SOVD data element resource: GET reads, POST writes (if writable).
class SovdDataAccessor : public SovdResource {
public:
    // Read-only accessor
    explicit SovdDataAccessor(std::string name, DataReader reader);
    // Read-write accessor
    SovdDataAccessor(std::string name, DataReader reader, DataWriter writer);

    HttpResponse handleGet(const HttpRequest& req) override;
    HttpResponse handlePost(const HttpRequest& req) override;

private:
    std::string  name_;
    DataReader   reader_;
    DataWriter   writer_;   // empty if read-only
};

} // namespace sdv::sovd
