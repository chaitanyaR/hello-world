#include <sovd_server/HpcSovdResource.hpp>
#include <gtest/gtest.h>
TEST(HpcSovdResource, ConstructsWithoutCrash) {
    sdv::hpc::sovd_server::HpcSovdResource res;
    (void)res;
}
