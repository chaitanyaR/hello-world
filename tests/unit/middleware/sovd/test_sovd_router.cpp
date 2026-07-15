#include <sovd/SovdRouter.hpp>
#include <sovd/SovdAuthentication.hpp>
#include <sovd/SovdFaultMemory.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace sdv::sovd;

class RouterTest : public ::testing::Test {
protected:
    SovdAuthentication auth;
    SovdRouter         router{auth};

    void SetUp() override {
        auth.registerToken("test-token", SovdRole::Technician);
        auto fm = std::make_shared<SovdFaultMemory>();
        router.mount("/api/sovd/v1/zones/powertrain/faults", fm);
    }
};

TEST_F(RouterTest, Returns404ForUnknownPath) {
    HttpRequest req;
    req.method = "GET";
    req.path   = "/api/sovd/v1/unknown";
    req.query["Authorization"] = "test-token";
    auto resp = router.dispatch(req);
    EXPECT_EQ(resp.statusCode, 404);
}

TEST_F(RouterTest, Returns401WithoutToken) {
    HttpRequest req;
    req.method = "GET";
    req.path   = "/api/sovd/v1/zones/powertrain/faults";
    auto resp = router.dispatch(req);
    EXPECT_EQ(resp.statusCode, 401);
}

TEST_F(RouterTest, Returns200WithValidToken) {
    HttpRequest req;
    req.method = "GET";
    req.path   = "/api/sovd/v1/zones/powertrain/faults";
    req.query["Authorization"] = "test-token";
    auto resp = router.dispatch(req);
    EXPECT_EQ(resp.statusCode, 200);
}
