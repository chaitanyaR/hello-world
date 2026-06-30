#include <someip/ServiceRegistry.hpp>
#include <gtest/gtest.h>

using namespace sdv::someip;

TEST(ServiceRegistry, ConstructsWithoutCrash) {
    EXPECT_NO_THROW(ServiceRegistry reg("test-app"));
}

TEST(ServiceRegistry, SubscribeAvailabilityFiresCallbackInSimMode) {
    ServiceRegistry reg("test-app-2");
    bool called = false;
    reg.subscribeAvailability(0x1001, 0x0001,
                              [&](bool avail) { called = avail; });
    // In simulation mode the stub immediately calls the callback with true
    EXPECT_TRUE(called);
}
