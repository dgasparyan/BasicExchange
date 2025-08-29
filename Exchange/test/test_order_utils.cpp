#include <gtest/gtest.h>
#include "OrderUtils.h"

namespace Exchange {
namespace test {

class OrderUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code that will be called before each test
    }

    void TearDown() override {
        // Cleanup code that will be called after each test
    }
};

TEST_F(OrderUtilsTest, ToSide) {
    EXPECT_EQ(toSide("BUY"), Side::Buy);
    EXPECT_EQ(toSide("  buy  "), Side::Buy);
    EXPECT_EQ(toSide("1"), Side::Buy);

    EXPECT_EQ(toSide("SELL"), Side::Sell);
    EXPECT_EQ(toSide("  sell  "), Side::Sell);
    EXPECT_EQ(toSide("2"), Side::Sell);

    EXPECT_EQ(toSide("   "), Side::Invalid);
    EXPECT_EQ(toSide("HOLD"), Side::Invalid);
    EXPECT_EQ(toSide("buying"), Side::Invalid);
}

TEST_F(OrderUtilsTest, ToType) {
    EXPECT_EQ(toType("MARKET"), Type::Market);
    EXPECT_EQ(toType("  market   "), Type::Market);
    EXPECT_EQ(toType("1"), Type::Market);

    EXPECT_EQ(toType("LIMIT"), Type::Limit);
    EXPECT_EQ(toType("  limit   "), Type::Limit);
    EXPECT_EQ(toType("2"), Type::Limit);

    EXPECT_EQ(toType("STOP"), Type::Invalid);
    EXPECT_EQ(toType("market order"), Type::Invalid);

}

} // namespace test
} // namespace Exchange 