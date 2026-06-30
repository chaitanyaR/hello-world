#pragma once
namespace sdv::zones::telemetry {
class V2xGateway {
public:
    bool isConnected() const;
    void setConnected(bool connected);
private:
    bool connected_{false};
};
}
