#include <telemetry/TelemetryServiceStub.hpp>

namespace sdv::zones::telemetry {

TelemetryServiceStub::TelemetryServiceStub(sdv::someip::ServiceRegistry& registry)
    : sdv::someip::ServiceStub(0x1003u, 0x0001u, registry)
{}

} // namespace sdv::zones::telemetry
