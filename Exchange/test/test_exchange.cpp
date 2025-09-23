#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <atomic>
#include <memory>
#include <thread>
#include <latch>

#include "Exchange.h"
#include "EventQueue.h"
#include "EventParser.h"
#include "OrderBookManager.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Throw;

namespace Exchange {
namespace test {

class MockSubscription : public SubscriptionHandle {
public:
  ~MockSubscription() override = default;
};

class MockEventQueue : public EventQueue {
public:
  MOCK_METHOD(std::unique_ptr<SubscriptionHandle>, subscribe, (MessageCallback), (override));
};

class MockEventParser : public EventParser {
public:
  MOCK_METHOD(EventType, getEventType, (std::string_view), (const, override));
  MOCK_METHOD(Event, parse, (std::string_view), (const, override));
};

class MockOrderBookManager : public IOrderBookManager {
public:
  MOCK_METHOD(SubmitResult, submit, (const Event&), (override));
  MOCK_METHOD(void, stop, (), (override));
};

// Utility to create Exchange with mocks and capture the subscription callback
struct ExchangeHarness {
  NiceMock<MockEventQueue> eventQueue;
  NiceMock<MockEventParser> parser;
  NiceMock<MockOrderBookManager> obm;
  EventQueue::MessageCallback cb;
  std::latch subscribed{1};

  std::unique_ptr<Exchange> makeExchange() {
    ON_CALL(eventQueue, subscribe(testing::_))
        .WillByDefault([this](EventQueue::MessageCallback cbIn) {
          cb = std::move(cbIn);
          subscribed.count_down();
          return std::make_unique<MockSubscription>();
        });
    return std::make_unique<Exchange>(eventQueue, parser, obm);
  }
};

TEST(ExchangeTest, ProcessesValidEventsThenStopsOnQuit) {
  ExchangeHarness h;
  auto ex = h.makeExchange();

  EXPECT_CALL(h.parser, getEventType("D, ..."))
      .WillOnce(Return(EventType::NewOrder));
  EXPECT_CALL(h.parser, parse("D, ..."))
      .WillOnce(Return(Event{std::in_place_type<NewOrderEvent>, "u"_uid, 1, "AAPL"_sym, 1, Side::Buy, Type::Market, MARKET_PRICE}));
  EXPECT_CALL(h.obm, submit(testing::_)).WillOnce(Return(SubmitResult::Success));

  EXPECT_CALL(h.parser, getEventType("Q"))
      .WillOnce(Return(EventType::Quit));
  EXPECT_CALL(h.obm, stop()).Times(testing::AtLeast(1));

  std::jthread t([&] { ex->start(); });
  h.subscribed.wait();
  h.cb("D, ...");
  h.cb("Q");

  t.join();
}

TEST(ExchangeTest, ContinuesOnParserException) {
  ExchangeHarness h;
  auto ex = h.makeExchange();

  EXPECT_CALL(h.parser, getEventType("D, bad"))
      .WillOnce(Return(EventType::NewOrder));
  EXPECT_CALL(h.parser, parse("D, bad")).WillOnce(Throw(std::runtime_error("parse error")));

  EXPECT_CALL(h.parser, getEventType("Q"))
      .WillOnce(Return(EventType::Quit));
  EXPECT_CALL(h.obm, stop()).Times(testing::AtLeast(1));

  std::jthread t([&] { ex->start(); });
  h.subscribed.wait();
  h.cb("D, bad");
  h.cb("Q");

  t.join();
}

} // namespace test
} // namespace Exchange

