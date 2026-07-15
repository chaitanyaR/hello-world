#pragma once
#include <cstdint>
namespace sdv::zones::telemetry {
class TelemetryPublisher {
public:
    void     publish();
    uint64_t publishCount() const;
private:
    uint64_t tickCount_{0};
};
}
