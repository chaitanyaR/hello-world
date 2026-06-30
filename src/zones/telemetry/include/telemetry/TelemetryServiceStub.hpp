#pragma once
#include <someip/ServiceStub.hpp>
namespace sdv::zones::telemetry {
class TelemetryServiceStub : public sdv::someip::ServiceStub {
public:
    TelemetryServiceStub(sdv::someip::ServiceRegistry& registry);
};
}
