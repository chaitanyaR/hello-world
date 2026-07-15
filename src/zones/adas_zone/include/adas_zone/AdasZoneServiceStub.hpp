#pragma once
#include <someip/ServiceStub.hpp>
namespace sdv::zones::adas_zone {
class AdasZoneServiceStub : public sdv::someip::ServiceStub {
public:
    AdasZoneServiceStub(sdv::someip::ServiceRegistry& registry);
};
}
