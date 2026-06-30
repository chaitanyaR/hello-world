#pragma once

#include "ServiceRegistry.hpp"

namespace sdv::someip {

// Base class for server-side SOME/IP stubs.
// Subclasses override onRequest() for each method and call fireEvent() to
// broadcast events to subscribed clients.
class ServiceStub {
public:
    ServiceStub(ServiceId serviceId, InstanceId instanceId,
                ServiceRegistry& registry);
    virtual ~ServiceStub();

    // Register the service with SOME/IP SD and start accepting requests.
    void offer();
    void withdraw();

protected:
    ServiceId        serviceId_;
    InstanceId       instanceId_;
    ServiceRegistry& registry_;
};

} // namespace sdv::someip
