#include <powertrain/PowertrainServiceStub.hpp>

namespace sdv::zones::powertrain {

PowertrainServiceStub::PowertrainServiceStub(sdv::someip::ServiceRegistry& registry)
    : sdv::someip::ServiceStub(0x1001u, 0x0001u, registry)
{}

} // namespace sdv::zones::powertrain
