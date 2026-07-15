#include "sovd/SovdFaultMemory.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>

namespace sdv::sovd {

void SovdFaultMemory::add(DtcEntry entry)
{
    std::lock_guard lock(mutex_);
    dtcs_.push_back(std::move(entry));
}

void SovdFaultMemory::clear(uint32_t dtcCode)
{
    std::lock_guard lock(mutex_);
    dtcs_.erase(std::remove_if(dtcs_.begin(), dtcs_.end(),
                               [dtcCode](const DtcEntry& e) {
                                   return e.dtcCode == dtcCode;
                               }),
                dtcs_.end());
}

void SovdFaultMemory::clearAll()
{
    std::lock_guard lock(mutex_);
    dtcs_.clear();
}

std::vector<DtcEntry> SovdFaultMemory::snapshot() const
{
    std::lock_guard lock(mutex_);
    return dtcs_;
}

HttpResponse SovdFaultMemory::handleGet(const HttpRequest&)
{
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& e : snapshot()) {
        arr.push_back({
            {"dtcCode",     e.dtcCode},
            {"description", e.description},
            {"severity",    e.severity},
            {"timestampUs", e.timestampUs}
        });
    }
    return ok(arr.dump());
}

HttpResponse SovdFaultMemory::handleDelete(const HttpRequest& req)
{
    // Path: /faults/{dtcCode}  → parse the last segment as hex or decimal
    const auto& path = req.path;
    auto pos = path.rfind('/');
    if (pos == std::string::npos || pos + 1 >= path.size()) {
        clearAll();
        return noContent();
    }
    try {
        uint32_t code = std::stoul(path.substr(pos + 1), nullptr, 0);
        clear(code);
        return noContent();
    } catch (...) {
        return error(400, "Bad Request", "Invalid DTC code in path");
    }
}

} // namespace sdv::sovd
