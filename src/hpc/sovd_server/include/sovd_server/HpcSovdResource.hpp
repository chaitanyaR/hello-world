#pragma once

#include <sovd/SovdResource.hpp>
#include <sovd/SovdFaultMemory.hpp>
#include <sovd/SovdDataAccessor.hpp>
#include <string>

namespace sdv::hpc::sovd_server {

// /hpc SOVD resource: exposes HPC-level data elements, faults, and routines.
// Sub-resources (AdasSovdResource, AdSovdResource) handle /hpc/adas and /hpc/ad.
class HpcSovdResource : public sdv::sovd::SovdResource {
public:
    HpcSovdResource();

    sdv::sovd::HttpResponse handleGet(const sdv::sovd::HttpRequest& req) override;

    // Call from the HPC process to push a new DTC into the fault memory.
    void reportFault(sdv::sovd::DtcEntry entry);

private:
    sdv::sovd::SovdFaultMemory faultMemory_;
};

} // namespace sdv::hpc::sovd_server
