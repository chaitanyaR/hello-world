#include "sovd/SovdServer.hpp"
#include <stdexcept>

// Pistache integration is gated behind its availability.
// In Phase 1 (no Pistache yet) the server compiles as a no-op stub.
#ifdef PISTACHE_AVAILABLE
#  include <pistache/endpoint.h>
#  include <pistache/router.h>
#endif

namespace sdv::sovd {

struct SovdServer::Impl {
    SovdServerConfig    config;
    SovdAuthentication  auth;
    SovdRouter          router{auth};

#ifdef PISTACHE_AVAILABLE
    std::unique_ptr<Pistache::Http::Endpoint> endpoint;
#endif
};

SovdServer::SovdServer(SovdServerConfig config)
    : impl_(std::make_unique<Impl>())
{
    impl_->config = std::move(config);
}

SovdServer::~SovdServer() { stop(); }

SovdRouter& SovdServer::router() { return impl_->router; }

SovdAuthentication& SovdServer::auth() { return impl_->auth; }

void SovdServer::start()
{
#ifdef PISTACHE_AVAILABLE
    Pistache::Address addr(impl_->config.bindAddress,
                           Pistache::Port(impl_->config.port));
    impl_->endpoint = std::make_unique<Pistache::Http::Endpoint>(addr);
    auto opts = Pistache::Http::Endpoint::options()
                    .threads(impl_->config.threadCount);
    impl_->endpoint->init(opts);
    impl_->endpoint->setHandler(
        Pistache::Http::make_handler</* handler class */ void>());
    impl_->endpoint->serveThreaded();
#endif
}

void SovdServer::stop()
{
#ifdef PISTACHE_AVAILABLE
    if (impl_->endpoint) impl_->endpoint->shutdown();
#endif
}

} // namespace sdv::sovd
