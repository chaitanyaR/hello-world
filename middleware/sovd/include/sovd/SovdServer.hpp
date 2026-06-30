#pragma once

#include "SovdRouter.hpp"
#include "SovdAuthentication.hpp"
#include <cstdint>
#include <memory>

namespace sdv::sovd {

struct SovdServerConfig {
    std::string bindAddress{"0.0.0.0"};
    uint16_t    port{8080};
    unsigned    threadCount{4};
    bool        tlsEnabled{false};
    std::string tlsCertPath;
    std::string tlsKeyPath;
};

// Pistache HTTP server entry point.
// Owns the router and authentication manager.
// Call start() to begin serving; stop() for graceful shutdown.
class SovdServer {
public:
    explicit SovdServer(SovdServerConfig config);
    ~SovdServer();

    SovdRouter&         router();
    SovdAuthentication& auth();

    void start();
    void stop();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace sdv::sovd
