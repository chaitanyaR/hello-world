#pragma once

#include "SovdResource.hpp"
#include <mutex>
#include <vector>

namespace sdv::sovd {

// In-memory DTC store implementing the SOVD /faults resource.
// Thread-safe; populated by zone-specific adapters via add().
class SovdFaultMemory : public SovdResource {
public:
    void add(DtcEntry entry);
    void clear(uint32_t dtcCode);
    void clearAll();

    std::vector<DtcEntry> snapshot() const;

    // GET  /faults       → JSON array of DtcEntry
    // DELETE /faults/{id} → 204 No Content
    HttpResponse handleGet(const HttpRequest& req) override;
    HttpResponse handleDelete(const HttpRequest& req) override;

private:
    mutable std::mutex      mutex_;
    std::vector<DtcEntry>   dtcs_;
};

} // namespace sdv::sovd
