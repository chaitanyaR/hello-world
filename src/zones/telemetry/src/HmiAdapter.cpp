#include <telemetry/HmiAdapter.hpp>

namespace sdv::zones::telemetry {

void HmiAdapter::sendCommand(const std::string& cmd)
{
    lastCmd_ = cmd;
}

const std::string& HmiAdapter::lastCommand() const { return lastCmd_; }

} // namespace sdv::zones::telemetry
