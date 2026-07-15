#include <sovd/SovdAuthentication.hpp>
#include <gtest/gtest.h>

using namespace sdv::sovd;

TEST(SovdAuthentication, UnknownTokenReturnsUnauthenticated) {
    SovdAuthentication auth;
    EXPECT_EQ(auth.authenticate("bad-token"), SovdRole::Unauthenticated);
}

TEST(SovdAuthentication, RegisteredTokenReturnsCorrectRole) {
    SovdAuthentication auth;
    auth.registerToken("tk-1", SovdRole::Technician);
    auth.registerToken("oem-1", SovdRole::OemSupport);
    EXPECT_EQ(auth.authenticate("tk-1"),  SovdRole::Technician);
    EXPECT_EQ(auth.authenticate("oem-1"), SovdRole::OemSupport);
}

TEST(SovdAuthentication, FaultsPathRequiresTechnician) {
    EXPECT_EQ(SovdAuthentication::minimumRoleForPath("/faults"),
              SovdRole::Technician);
}
