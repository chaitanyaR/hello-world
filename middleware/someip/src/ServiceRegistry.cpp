#include "someip/ServiceRegistry.hpp"

#ifdef SDV_SIMULATION_MODE
// In simulation mode vsomeip is optional; compile without it if not found.
#  ifdef VSOMEIP_AVAILABLE
#    include <vsomeip/vsomeip.hpp>
#  endif
#endif

#include <stdexcept>

namespace sdv::someip {

struct ServiceRegistry::Impl {
    std::string name;
#ifdef VSOMEIP_AVAILABLE
    std::shared_ptr<vsomeip::application> app;
#endif
};

ServiceRegistry::ServiceRegistry(const std::string& applicationName)
    : impl_(std::make_unique<Impl>())
{
    impl_->name = applicationName;
#ifdef VSOMEIP_AVAILABLE
    impl_->app = vsomeip::runtime::get()->create_application(applicationName);
    if (!impl_->app->init()) {
        throw std::runtime_error("vsomeip init failed for: " + applicationName);
    }
#endif
}

ServiceRegistry::~ServiceRegistry() = default;

void ServiceRegistry::start()
{
#ifdef VSOMEIP_AVAILABLE
    impl_->app->start();
#endif
}

void ServiceRegistry::stop()
{
#ifdef VSOMEIP_AVAILABLE
    impl_->app->stop();
#endif
}

void ServiceRegistry::subscribeAvailability(ServiceId serviceId,
                                             InstanceId instanceId,
                                             ServiceAvailabilityHandler handler)
{
#ifdef VSOMEIP_AVAILABLE
    impl_->app->register_availability_handler(
        serviceId, instanceId,
        [h = std::move(handler)](vsomeip::service_t, vsomeip::instance_t,
                                  bool available) { h(available); });
    impl_->app->request_service(serviceId, instanceId);
#else
    // Simulation stub: immediately signal available
    handler(true);
#endif
}

void ServiceRegistry::offerService(ServiceId serviceId, InstanceId instanceId)
{
#ifdef VSOMEIP_AVAILABLE
    impl_->app->offer_service(serviceId, instanceId);
#endif
}

void ServiceRegistry::stopOfferService(ServiceId serviceId, InstanceId instanceId)
{
#ifdef VSOMEIP_AVAILABLE
    impl_->app->stop_offer_service(serviceId, instanceId);
#endif
}

void* ServiceRegistry::nativeApplication() const
{
#ifdef VSOMEIP_AVAILABLE
    return impl_->app.get();
#else
    return nullptr;
#endif
}

} // namespace sdv::someip
