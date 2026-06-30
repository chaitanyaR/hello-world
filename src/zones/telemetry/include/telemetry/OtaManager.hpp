#pragma once

#include <atomic>
#include <functional>
#include <string>
#include <thread>

namespace sdv::zones::telemetry {

enum class OtaState { Idle, Downloading, Verifying, Installing, Done, Failed };

using OtaStatusCallback = std::function<void(OtaState, uint8_t progress)>;

// OTA update FSM: IDLE → DOWNLOADING → VERIFYING → INSTALLING → DONE/FAILED.
// All state transitions happen on a background thread.
class OtaManager {
public:
    explicit OtaManager(OtaStatusCallback statusCb);
    ~OtaManager();

    // Returns false if an update is already in progress.
    bool trigger(const std::string& packageUrl, const std::string& sha256Hash);

    OtaState currentState() const;
    uint8_t  progress() const;

private:
    void run(std::string url, std::string hash);

    OtaStatusCallback        statusCb_;
    std::atomic<OtaState>    state_{OtaState::Idle};
    std::atomic<uint8_t>     progress_{0};
    std::thread              worker_;
};

} // namespace sdv::zones::telemetry
