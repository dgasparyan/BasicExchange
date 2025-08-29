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
    auto event = std::make_unique<NewOrderEvent>("user1", 1, "AAPL", 100, Side::Buy, Type::Limit, 150.50);
    
    EXPECT_EQ(event->type(), EventType::NewOrder);
    EXPECT_EQ(event->userId_, "user1");
    EXPECT_EQ(event->clientOrderId_, 1);
    EXPECT_EQ(event->symbol_, "AAPL");
    EXPECT_EQ(event->quantity_, 100);
    EXPECT_EQ(event->side_, Side::Buy);
    EXPECT_EQ(event->type_, Type::Limit);
    EXPECT_EQ(event->price_, 150.50);
}

TEST_F(EventTest, NewOrderEvent_DefaultPrice) {
    auto event = std::make_unique<NewOrderEvent>("user1", 1, "AAPL", 100, Side::Buy, Type::Limit);
    
    EXPECT_EQ(event->price_, INVALID_PRICE);
}

TEST_F(EventTest, CancelOrderEvent_Construction) {
    auto event = std::make_unique<CancelOrderEvent>("user123", 1001, "AAPL");
    
    EXPECT_EQ(event->type(), EventType::CancelOrder);
    EXPECT_EQ(event->userId_, "user123");
    EXPECT_EQ(event->origOrderId_, 1001);
    EXPECT_EQ(event->symbol_, "AAPL");
}

TEST_F(EventTest, TopOfBookEvent_Construction) {
    auto event = std::make_unique<TopOfBookEvent>("user123", "AAPL");
    
    EXPECT_EQ(event->type(), EventType::TopOfBook);
    EXPECT_EQ(event->userId_, "user123");
    EXPECT_EQ(event->symbol_, "AAPL");
}

TEST_F(EventTest, QuitEvent_Construction) {
    auto event = std::make_unique<QuitEvent>();
    
    EXPECT_EQ(event->type(), EventType::Quit);
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

TEST_F(EventTest, ToString_ValidTypes) {
    EXPECT_EQ(toString(EventType::NewOrder), "NewOrder");
    EXPECT_EQ(toString(EventType::CancelOrder), "CancelOrder");
    EXPECT_EQ(toString(EventType::TopOfBook), "TopOfBook");
    EXPECT_EQ(toString(EventType::Quit), "Quit");
    EXPECT_EQ(toString(EventType::Invalid), "Invalid");
}

TEST_F(EventTest, EventTraits) {
    EXPECT_STREQ(EventTraits<EventType::NewOrder>::name, "NewOrder");
    EXPECT_STREQ(EventTraits<EventType::CancelOrder>::name, "CancelOrder");
    EXPECT_STREQ(EventTraits<EventType::TopOfBook>::name, "TopOfBook");
    EXPECT_STREQ(EventTraits<EventType::Quit>::name, "Quit");
}

} // namespace test
} // namespace Exchange 