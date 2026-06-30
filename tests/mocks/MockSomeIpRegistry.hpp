#pragma once
#include <someip/ServiceRegistry.hpp>
#include <gmock/gmock.h>

namespace sdv::test {
class MockServiceRegistry : public someip::ServiceRegistry {
public:
    MockServiceRegistry() : someip::ServiceRegistry("mock") {}
    MOCK_METHOD(void, subscribeAvailability,
                (someip::ServiceId, someip::InstanceId,
                 someip::ServiceAvailabilityHandler), ());
    MOCK_METHOD(void, offerService,
                (someip::ServiceId, someip::InstanceId), ());
    MOCK_METHOD(void, stopOfferService,
                (someip::ServiceId, someip::InstanceId), ());
};
}
