#include <telemetry/V2xGateway.hpp>

namespace sdv::zones::telemetry {

bool V2xGateway::isConnected() const { return connected_; }

void V2xGateway::setConnected(bool connected) { connected_ = connected; }

} // namespace sdv::zones::telemetry
