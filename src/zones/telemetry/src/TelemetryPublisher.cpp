#include <telemetry/TelemetryPublisher.hpp>
#include <iostream>

namespace sdv::zones::telemetry {

void TelemetryPublisher::publish()
{
    ++tickCount_;
    // In a real system this would serialize and send over SOME/IP.
    // Here we just increment the counter so tests can verify it was called.
}

uint64_t TelemetryPublisher::publishCount() const { return tickCount_; }

} // namespace sdv::zones::telemetry
