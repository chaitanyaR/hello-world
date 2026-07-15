#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace sdv::someip {

using ServiceId   = uint16_t;
using InstanceId  = uint16_t;
using ServiceAvailabilityHandler = std::function<void(bool available)>;

// Wraps vsomeip application lifecycle and SOME/IP Service Discovery.
// Each process creates exactly one ServiceRegistry and calls start() before
// offering or consuming any service.
class ServiceRegistry {
public:
    explicit ServiceRegistry(const std::string& applicationName);
    ~ServiceRegistry();

    ServiceRegistry(const ServiceRegistry&) = delete;
    ServiceRegistry& operator=(const ServiceRegistry&) = delete;

    void start();
    void stop();

    // Subscribe to SD availability notifications for a service.
    void subscribeAvailability(ServiceId serviceId, InstanceId instanceId,
                               ServiceAvailabilityHandler handler);

    // Offer a local service (called by stubs).
    void offerService(ServiceId serviceId, InstanceId instanceId);
    void stopOfferService(ServiceId serviceId, InstanceId instanceId);

    // Access the underlying vsomeip application pointer (opaque to callers).
    void* nativeApplication() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace sdv::someip
