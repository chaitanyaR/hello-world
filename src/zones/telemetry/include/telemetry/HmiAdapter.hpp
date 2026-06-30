#pragma once
#include <string>
namespace sdv::zones::telemetry {
class HmiAdapter {
public:
    void               sendCommand(const std::string& cmd);
    const std::string& lastCommand() const;
private:
    std::string lastCmd_;
};
}
