#include <sovd/SovdFaultMemory.hpp>
#include <gtest/gtest.h>

using namespace sdv::sovd;

TEST(SovdFaultMemory, StartsEmpty) {
    SovdFaultMemory fm;
    EXPECT_TRUE(fm.snapshot().empty());
}

TEST(SovdFaultMemory, AddAndSnapshot) {
    SovdFaultMemory fm;
    fm.add({0xA001, "Overvoltage", "error", 1000, 0.0f});
    fm.add({0xA002, "Overcurrent", "warning", 2000, 0.0f});
    EXPECT_EQ(fm.snapshot().size(), 2u);
}

TEST(SovdFaultMemory, ClearByCode) {
    SovdFaultMemory fm;
    fm.add({0xA001, "Overvoltage", "error", 1000, 0.0f});
    fm.add({0xA002, "Overcurrent", "warning", 2000, 0.0f});
    fm.clear(0xA001);
    auto snap = fm.snapshot();
    EXPECT_EQ(snap.size(), 1u);
    EXPECT_EQ(snap[0].dtcCode, 0xA002u);
}

TEST(SovdFaultMemory, ClearAll) {
    SovdFaultMemory fm;
    fm.add({0xA001, "Overvoltage", "error", 1000, 0.0f});
    fm.clearAll();
    EXPECT_TRUE(fm.snapshot().empty());
}

TEST(SovdFaultMemory, GetReturns200WithJsonArray) {
    SovdFaultMemory fm;
    fm.add({0xA001, "Overvoltage", "error", 1000, 0.0f});
    HttpRequest req;
    req.method = "GET";
    req.path   = "/faults";
    auto resp = fm.handleGet(req);
    EXPECT_EQ(resp.statusCode, 200);
    // DTC code is serialized as a decimal integer (40961 == 0xA001)
    EXPECT_NE(resp.body.find("40961"), std::string::npos);
}

TEST(SovdFaultMemory, DeleteClearsByCode) {
    SovdFaultMemory fm;
    fm.add({0xA001, "Overvoltage", "error", 1000, 0.0f});
    HttpRequest req;
    req.method = "DELETE";
    req.path   = "/faults/0xA001";
    auto resp = fm.handleDelete(req);
    EXPECT_EQ(resp.statusCode, 204);
    EXPECT_TRUE(fm.snapshot().empty());
}
