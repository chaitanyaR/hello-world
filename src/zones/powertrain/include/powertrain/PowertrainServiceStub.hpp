#pragma once
#include <someip/ServiceStub.hpp>
namespace sdv::zones::powertrain {
class PowertrainServiceStub : public sdv::someip::ServiceStub {
public:
    PowertrainServiceStub(sdv::someip::ServiceRegistry& registry);
};
}
