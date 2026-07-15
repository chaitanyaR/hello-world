#include <body/BodyServiceStub.hpp>

namespace sdv::zones::body {

BodyServiceStub::BodyServiceStub(sdv::someip::ServiceRegistry& registry)
    : sdv::someip::ServiceStub(0x1004u, 0x0001u, registry)
{}

} // namespace sdv::zones::body
