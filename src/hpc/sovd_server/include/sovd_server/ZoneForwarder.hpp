#pragma once

#include <sovd/SovdResource.hpp>
#include <someip/ServiceProxy.hpp>
#include <string>

namespace sdv::hpc::sovd_server {

// Facade: translates SOVD REST calls for /zones/{zone}/* into
// DiagnosticService (0x1FFF) SOME/IP method calls on the target zone ECU.
// Each zone gets its own ZoneForwarder instance, mounted at its path prefix.
class ZoneForwarder : public sdv::sovd::SovdResource {
public:
    ZoneForwarder(std::string zoneName,
                  sdv::someip::ServiceId diagnosticServiceId,
                  sdv::someip::InstanceId instanceId,
                  sdv::someip::ServiceRegistry& registry);

    sdv::sovd::HttpResponse handleGet(const sdv::sovd::HttpRequest& req) override;
    sdv::sovd::HttpResponse handlePost(const sdv::sovd::HttpRequest& req) override;
    sdv::sovd::HttpResponse handleDelete(const sdv::sovd::HttpRequest& req) override;

private:
    std::string                             zoneName_;
    sdv::someip::ServiceId                  serviceId_;
    sdv::someip::InstanceId                 instanceId_;
    sdv::someip::ServiceRegistry&           registry_;
};

} // namespace sdv::hpc::sovd_server
