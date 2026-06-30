#pragma once
#include <sovd/SovdResource.hpp>
namespace sdv::hpc::sovd_server {
class AdasSovdResource : public sdv::sovd::SovdResource {
public:
    sdv::sovd::HttpResponse handleGet(const sdv::sovd::HttpRequest& req) override;
};
}
