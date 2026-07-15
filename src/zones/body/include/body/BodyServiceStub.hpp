#pragma once
#include <someip/ServiceStub.hpp>
namespace sdv::zones::body {
class BodyServiceStub : public sdv::someip::ServiceStub {
public:
    BodyServiceStub(sdv::someip::ServiceRegistry& registry);
};
}
