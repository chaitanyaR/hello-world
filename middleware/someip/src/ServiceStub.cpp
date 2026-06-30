#include "someip/ServiceStub.hpp"

namespace sdv::someip {

ServiceStub::ServiceStub(ServiceId serviceId, InstanceId instanceId,
                         ServiceRegistry& registry)
    : serviceId_(serviceId), instanceId_(instanceId), registry_(registry)
{}

ServiceStub::~ServiceStub() { withdraw(); }

void ServiceStub::offer()
{
    registry_.offerService(serviceId_, instanceId_);
}

void ServiceStub::withdraw()
{
    registry_.stopOfferService(serviceId_, instanceId_);
}

} // namespace sdv::someip
