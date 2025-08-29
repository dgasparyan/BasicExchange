#include <gtest/gtest.h>
#include "CommonUtils.h"

namespace Exchange {
namespace test {

class CommonUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code that will be called before each test
    }

    void TearDown() override {
        // Cleanup code that will be called after each test
    }
};

TEST_F(CommonUtilsTest, TrimCopy_EmptyString) {
    EXPECT_EQ(trimCopy(""), "");
    EXPECT_EQ(trimCopy("   "), "");
    EXPECT_EQ(trimCopy("\t\r\n"), "");
}

TEST_F(CommonUtilsTest, TrimCopy_NoWhitespace) {
    EXPECT_EQ(trimCopy("hello"), "hello");
    EXPECT_EQ(trimCopy("world"), "world");
}

TEST_F(CommonUtilsTest, TrimCopy_LeadingWhitespace) {
    EXPECT_EQ(trimCopy("  hello"), "hello");
    EXPECT_EQ(trimCopy("\t  world"), "world");
}

TEST_F(CommonUtilsTest, TrimCopy_TrailingWhitespace) {
    EXPECT_EQ(trimCopy("hello  "), "hello");
    EXPECT_EQ(trimCopy("world\t"), "world");
}

TEST_F(CommonUtilsTest, TrimCopy_BothSides) {
    EXPECT_EQ(trimCopy("  hello world  "), "hello world");
    EXPECT_EQ(trimCopy("\t\r\n test \r\n\t"), "test");
}

TEST_F(CommonUtilsTest, TrimCopy_CustomSeparators) {
    EXPECT_EQ(trimCopy("..hello..", ".,"), "hello");
    EXPECT_EQ(trimCopy(";;test;;", ";"), "test");
    EXPECT_EQ(trimCopy("abc123", "abc"), "123");
}

TEST_F(CommonUtilsTest, TrimAndUpperCopy_EmptyString) {
    EXPECT_EQ(trimAndUpperCopy(""), "");
    EXPECT_EQ(trimAndUpperCopy("   "), "");
}

TEST_F(CommonUtilsTest, TrimAndUpperCopy_Basic) {
    EXPECT_EQ(trimAndUpperCopy("hello"), "HELLO");
    EXPECT_EQ(trimAndUpperCopy("world"), "WORLD");
}

TEST_F(CommonUtilsTest, TrimAndUpperCopy_WithWhitespace) {
    EXPECT_EQ(trimAndUpperCopy("  hello world  "), "HELLO WORLD");
    EXPECT_EQ(trimAndUpperCopy("\t\r\n test \r\n\t"), "TEST");
}

TEST_F(CommonUtilsTest, TrimAndUpperCopy_MixedCase) {
    EXPECT_EQ(trimAndUpperCopy("Hello World"), "HELLO WORLD");
    EXPECT_EQ(trimAndUpperCopy("MiXeD cAsE"), "MIXED CASE");
}

TEST_F(CommonUtilsTest, TrimAndUpperCopy_CustomSeparators) {
    EXPECT_EQ(trimAndUpperCopy("..hello.world..", ".,"), "HELLO.WORLD");
    EXPECT_EQ(trimAndUpperCopy(";;test;;", ";"), "TEST");
}

} // namespace test
} // namespace Exchange 