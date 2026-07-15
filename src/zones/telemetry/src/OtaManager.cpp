#include <telemetry/OtaManager.hpp>
#include <chrono>
#include <thread>

namespace sdv::zones::telemetry {

OtaManager::OtaManager(OtaStatusCallback statusCb)
    : statusCb_(std::move(statusCb))
{}

OtaManager::~OtaManager()
{
    if (worker_.joinable()) worker_.join();
}

bool OtaManager::trigger(const std::string& packageUrl, const std::string& sha256Hash)
{
    OtaState current = state_.load();
    if (current != OtaState::Idle && current != OtaState::Done &&
        current != OtaState::Failed) {
        return false;
    }
    if (worker_.joinable()) worker_.join();
    worker_ = std::thread(&OtaManager::run, this, packageUrl, sha256Hash);
    return true;
}

OtaState OtaManager::currentState() const { return state_.load(); }
uint8_t  OtaManager::progress()     const { return progress_.load(); }

void OtaManager::run(std::string /*url*/, std::string /*hash*/)
{
    auto notify = [this](OtaState s, uint8_t p) {
        state_    = s;
        progress_ = p;
        if (statusCb_) statusCb_(s, p);
    };

    notify(OtaState::Downloading, 0);
    for (uint8_t p = 0; p <= 100; p += 20) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        notify(OtaState::Downloading, p);
    }

    notify(OtaState::Verifying, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    notify(OtaState::Installing, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    notify(OtaState::Done, 100);
}

} // namespace sdv::zones::telemetry
