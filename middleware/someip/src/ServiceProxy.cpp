#include "someip/ServiceProxy.hpp"
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace sdv::someip {

ServiceProxy::ServiceProxy(ServiceId serviceId, InstanceId instanceId,
                           ServiceRegistry& registry)
    : serviceId_(serviceId), instanceId_(instanceId), registry_(registry)
{
    registry_.subscribeAvailability(serviceId, instanceId,
        [this](bool available) { onAvailabilityChanged(available); });
}

ServiceProxy::~ServiceProxy() = default;

bool ServiceProxy::isAvailable() const
{
    return available_.load(std::memory_order_acquire);
}

bool ServiceProxy::waitAvailable(unsigned timeoutMs)
{
    std::mutex mtx;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock(mtx);
    return cv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                       [this] { return available_.load(); });
}

void ServiceProxy::onAvailabilityChanged(bool available)
{
    available_.store(available, std::memory_order_release);
}

} // namespace sdv::someip
