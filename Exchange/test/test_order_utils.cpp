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

TEST_F(OrderUtilsTest, ToPrice_ValidConversions) {
    PriceSpec spec{100, 1}; // $0.01 scale, $0.01 tick
    
    EXPECT_EQ(toPrice(10.50, spec).ticks, 1050);
    EXPECT_EQ(toPrice(0.01, spec).ticks, 1);
    EXPECT_EQ(toPrice(0.00, spec).ticks, 0);
    EXPECT_EQ(toPrice(100.00, spec).ticks, 10000);
    EXPECT_EQ(toPrice(0.99, spec).ticks, 99);
}

TEST_F(OrderUtilsTest, ToPrice_DifferentSpecs) {
    PriceSpec twoDigitSpec{100, 1}; // $0.01 scale, $0.01 tick
    PriceSpec fiveDigitSpec{100000, 1}; // $0.00001 scale, $0.00001 tick
    PriceSpec tickSpec{100, 5}; // $0.01 scale, $0.05 tick
    
    EXPECT_EQ(toPrice(10.50, twoDigitSpec).ticks, 1050);
    EXPECT_EQ(toPrice(10.50000, fiveDigitSpec).ticks, 1050000);
    EXPECT_EQ(toPrice(10.50, tickSpec).ticks, 210); // 10.50 / 0.05 = 210
}

TEST_F(OrderUtilsTest, ToPrice_Rounding) {
    PriceSpec spec{100, 1}; // $0.01 scale, $0.01 tick
    
    EXPECT_EQ(toPrice(10.505, spec).ticks, 1051); // rounds to nearest
    EXPECT_EQ(toPrice(10.504, spec).ticks, 1050); // rounds down
    EXPECT_EQ(toPrice(10.506, spec).ticks, 1051); // rounds up
}

TEST_F(OrderUtilsTest, ToPrice_InvalidTickGrid) {
    PriceSpec spec{100, 5}; // $0.01 scale, $0.05 tick
    
    EXPECT_THROW(toPrice(10.01, spec), std::invalid_argument); // not on $0.05 grid
    EXPECT_THROW(toPrice(10.02, spec), std::invalid_argument); // not on $0.05 grid
    EXPECT_THROW(toPrice(10.03, spec), std::invalid_argument); // not on $0.05 grid
    EXPECT_THROW(toPrice(10.04, spec), std::invalid_argument); // not on $0.05 grid
    
    EXPECT_NO_THROW(toPrice(10.00, spec)); // on $0.05 grid
    EXPECT_NO_THROW(toPrice(10.05, spec)); // on $0.05 grid
    EXPECT_NO_THROW(toPrice(10.10, spec)); // on $0.05 grid
}

TEST_F(OrderUtilsTest, ToPrice_EdgeCases) {
    PriceSpec spec{100, 1}; // $0.01 scale, $0.01 tick
    
    EXPECT_EQ(toPrice(0.0, spec).ticks, 0);
    EXPECT_EQ(toPrice(-0.01, spec).ticks, -1);
    EXPECT_EQ(toPrice(-10.50, spec).ticks, -1050);
}

TEST_F(OrderUtilsTest, ToPrice_LargeNumbers) {
    PriceSpec spec{100, 1}; // $0.01 scale, $0.01 tick
    
    EXPECT_EQ(toPrice(999999.99, spec).ticks, 99999999);
    EXPECT_EQ(toPrice(1000000.00, spec).ticks, 100000000);
}

} // namespace test
} // namespace Exchange 