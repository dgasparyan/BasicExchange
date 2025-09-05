#include <gtest/gtest.h>
#include "Event.h"

namespace Exchange {
namespace test {

class EventTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code that will be called before each test
    }

    void TearDown() override {
        // Cleanup code that will be called after each test
    }
};


TEST_F(EventTest, NewOrderEvent_Construction) {
    NewOrderEvent event("user1"_uid, 1001, "AAPL"_sym, 100, Side::Buy, Type::Limit, toPrice(150.50, TWO_DIGITS_PRICE_SPEC));
    
    EXPECT_EQ(event.eventType(), EventType::NewOrder);
    EXPECT_EQ(event.userId_, "user1"_uid);
    EXPECT_EQ(event.clientOrderId_, 1001);
    EXPECT_EQ(event.symbol_, "AAPL"_sym);
    EXPECT_EQ(event.quantity_, 100);
    EXPECT_EQ(event.side_, Side::Buy);
    EXPECT_EQ(event.type_, Type::Limit);
    EXPECT_EQ(event.price_, toPrice(150.50, TWO_DIGITS_PRICE_SPEC));
}

TEST_F(EventTest, NewOrderEvent_DefaultPrice) {
    NewOrderEvent event("user1"_uid, 1001, "AAPL"_sym, 100, Side::Buy, Type::Limit);
    
    EXPECT_EQ(event.price_, INVALID_PRICE);
}

TEST_F(EventTest, CancelOrderEvent_Construction) {
    CancelOrderEvent event("user123"_uid, 1001, "AAPL"_sym, 2001);
    
    EXPECT_EQ(event.eventType(), EventType::CancelOrder);
    EXPECT_EQ(event.userId_, "user123"_uid);
    EXPECT_EQ(event.clientOrderId_, 1001);
    EXPECT_EQ(event.symbol_, "AAPL"_sym);
    EXPECT_EQ(event.origOrderId_, 2001);
}

TEST_F(EventTest, TopOfBookEvent_Construction) {
    TopOfBookEvent event("user123"_uid, 1001, "AAPL"_sym);
    
    EXPECT_EQ(event.eventType(), EventType::TopOfBook);
    EXPECT_EQ(event.userId_, "user123"_uid);
    EXPECT_EQ(event.clientOrderId_, 1001);
    EXPECT_EQ(event.symbol_, "AAPL"_sym);
}


TEST_F(EventTest, QuitEvent_Construction) {
    QuitEvent  event;
    EXPECT_EQ(event.eventType(), EventType::Quit);
}

TEST_F(EventTest, ToEventType_ValidTypes) {
    EXPECT_EQ(toEventType("D"), EventType::NewOrder);
    EXPECT_EQ(toEventType("d"), EventType::NewOrder);
    EXPECT_EQ(toEventType("  D  "), EventType::NewOrder);
    
    EXPECT_EQ(toEventType("F"), EventType::CancelOrder);
    EXPECT_EQ(toEventType("f"), EventType::CancelOrder);
    EXPECT_EQ(toEventType("  F  "), EventType::CancelOrder);
    
    EXPECT_EQ(toEventType("V"), EventType::TopOfBook);
    EXPECT_EQ(toEventType("v"), EventType::TopOfBook);
    EXPECT_EQ(toEventType("  V  "), EventType::TopOfBook);
    
    EXPECT_EQ(toEventType("Q"), EventType::Quit);
    EXPECT_EQ(toEventType("q"), EventType::Quit);
    EXPECT_EQ(toEventType("QUIT"), EventType::Quit);
    EXPECT_EQ(toEventType("quit"), EventType::Quit);
    EXPECT_EQ(toEventType("  Q  "), EventType::Quit);
}

TEST_F(EventTest, ToEventType_InvalidTypes) {
    EXPECT_EQ(toEventType(""), EventType::Invalid);
    EXPECT_EQ(toEventType("   "), EventType::Invalid);
    EXPECT_EQ(toEventType("X"), EventType::Invalid);
    EXPECT_EQ(toEventType("NEW"), EventType::Invalid);
    EXPECT_EQ(toEventType("CANCEL"), EventType::Invalid);
}

} // namespace test


} // namespace Exchange 