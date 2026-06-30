#include <adas_zone/AdasZoneServiceStub.hpp>

namespace sdv::zones::adas_zone {

AdasZoneServiceStub::AdasZoneServiceStub(sdv::someip::ServiceRegistry& registry)
    : sdv::someip::ServiceStub(0x1002u, 0x0001u, registry)
{}

} // namespace sdv::zones::adas_zone
