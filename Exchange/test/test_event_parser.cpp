#include <gtest/gtest.h>
#include "EventParser.h"

#include <list>

namespace Exchange {

class EventParserTest : public ::testing::Test {
  protected:
    std::unique_ptr<Exchange::CsvEventParser> parser;
protected:
    void SetUp() override {
        parser = std::make_unique<CsvEventParser>();
    }

    void TearDown() override {
        // Cleanup code that will be called after each test
    }

};

TEST_F(EventParserTest, ParseNewOrder_ValidMarketOrder) {
    std::string csv = "D,user123,1001,AAPL,100,BUY,MARKET";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::NewOrder);
    
    auto newOrderEvent = dynamic_cast<NewOrderEvent*>(event.get());
    ASSERT_NE(newOrderEvent, nullptr);
    EXPECT_EQ(newOrderEvent->userId_, "user123");
    EXPECT_EQ(newOrderEvent->clientOrderId_, 1001);
    EXPECT_EQ(newOrderEvent->symbol_, "AAPL");
    EXPECT_EQ(newOrderEvent->quantity_, 100);
    EXPECT_EQ(newOrderEvent->side_, Side::Buy);
    EXPECT_EQ(newOrderEvent->type_, Type::Market);
    EXPECT_EQ(newOrderEvent->price_, INVALID_PRICE);
}

TEST_F(EventParserTest, ParseNewOrder_ValidLimitOrder) {
    std::string csv = "D,user456,1002,MSFT,50,SELL,LIMIT,150.75";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::NewOrder);
    
    auto newOrderEvent = dynamic_cast<NewOrderEvent*>(event.get());
    ASSERT_NE(newOrderEvent, nullptr);
    EXPECT_EQ(newOrderEvent->userId_, "user456");
    EXPECT_EQ(newOrderEvent->clientOrderId_, 1002);
    EXPECT_EQ(newOrderEvent->symbol_, "MSFT");
    EXPECT_EQ(newOrderEvent->quantity_, 50);
    EXPECT_EQ(newOrderEvent->side_, Side::Sell);
    EXPECT_EQ(newOrderEvent->type_, Type::Limit);
    EXPECT_DOUBLE_EQ(newOrderEvent->price_, 150.75);
}

TEST_F(EventParserTest, ParseNewOrder_WithWhitespace) {
    std::string csv = " D , user789 , 1003 , GOOGL , 200 , BUY , MARKET ";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::NewOrder);
    
    auto newOrderEvent = dynamic_cast<NewOrderEvent*>(event.get());
    ASSERT_NE(newOrderEvent, nullptr);
    EXPECT_EQ(newOrderEvent->userId_, "user789");
    EXPECT_EQ(newOrderEvent->clientOrderId_, 1003);
    EXPECT_EQ(newOrderEvent->symbol_, "GOOGL");
    EXPECT_EQ(newOrderEvent->quantity_, 200);
    EXPECT_EQ(newOrderEvent->side_, Side::Buy);
    EXPECT_EQ(newOrderEvent->type_, Type::Market);
}

TEST_F(EventParserTest, ParseNewOrder_CaseInsensitive) {
    std::string csv = "d,user101,1004,TSLA,75,sell,limit,250.50";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::NewOrder);
    
    auto newOrderEvent = dynamic_cast<NewOrderEvent*>(event.get());
    ASSERT_NE(newOrderEvent, nullptr);
    EXPECT_EQ(newOrderEvent->userId_, "user101");
    EXPECT_EQ(newOrderEvent->clientOrderId_, 1004);
    EXPECT_EQ(newOrderEvent->symbol_, "TSLA");
    EXPECT_EQ(newOrderEvent->quantity_, 75);
    EXPECT_EQ(newOrderEvent->side_, Side::Sell);
    EXPECT_EQ(newOrderEvent->type_, Type::Limit);
    EXPECT_DOUBLE_EQ(newOrderEvent->price_, 250.50);
}

TEST_F(EventParserTest, ParseNewOrder_ShortForm) {
    std::string csv = "D,user202,1005,NFLX,25,1,2,180.25";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::NewOrder);
    
    auto newOrderEvent = dynamic_cast<NewOrderEvent*>(event.get());
    ASSERT_NE(newOrderEvent, nullptr);
    EXPECT_EQ(newOrderEvent->userId_, "user202");
    EXPECT_EQ(newOrderEvent->clientOrderId_, 1005);
    EXPECT_EQ(newOrderEvent->symbol_, "NFLX");
    EXPECT_EQ(newOrderEvent->quantity_, 25);
    EXPECT_EQ(newOrderEvent->side_, Side::Buy);
    EXPECT_EQ(newOrderEvent->type_, Type::Limit);
    EXPECT_DOUBLE_EQ(newOrderEvent->price_, 180.25);
}

TEST_F(EventParserTest, ParseNewOrder_InvalidSide) {
    std::string csv = "D,user404,1007,META,50,HOLD,MARKET";
    
    EXPECT_THROW(parser->parse(csv), std::runtime_error);
}

TEST_F(EventParserTest, ParseNewOrder_InvalidType) {
    std::string csv = "D,user505,1008,NVDA,100,BUY,STOP";
    
    EXPECT_THROW(parser->parse(csv), std::runtime_error);
}

TEST_F(EventParserTest, ParseNewOrder_LimitOrderMissingPrice) {
    std::string csv = "D,user606,1009,AMD,75,SELL,LIMIT";
    
    EXPECT_THROW(parser->parse(csv), std::runtime_error);
}

TEST_F(EventParserTest, ParseNewOrder_TooFewTokens) {
    std::string csv = "D,user707,1010,AAPL,100";
    
    EXPECT_THROW(parser->parse(csv), std::runtime_error);
}

TEST_F(EventParserTest, ParseNewOrder_WhitespaceOnly) {
    std::string csv = "   ";
    
    EXPECT_THROW(parser->parse(csv), std::runtime_error);
}

TEST_F(EventParserTest, ParseNewOrder_InvalidQuantity) {
    std::string csv = "D,user808,1011,INTC,abc,BUY,MARKET";
    
    EXPECT_THROW(parser->parse(csv), std::invalid_argument);
}

TEST_F(EventParserTest, ParseNewOrder_InvalidClientOrderId) {
    std::string csv = "D,user909,xyz,CRM,100,BUY,MARKET";
    
    EXPECT_THROW(parser->parse(csv), std::invalid_argument);
}

TEST_F(EventParserTest, ParseNewOrder_InvalidPrice) {
    std::string csv = "D,user1010,1012,ORCL,50,SELL,LIMIT,invalid";
    
    EXPECT_THROW(parser->parse(csv), std::invalid_argument);
}

TEST_F(EventParserTest, ParseNewOrder_ZeroQuantity) {
    std::string csv = "D,user1111,1013,ADBE,0,BUY,MARKET";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::NewOrder);
    
    auto newOrderEvent = dynamic_cast<NewOrderEvent*>(event.get());
    ASSERT_NE(newOrderEvent, nullptr);
    EXPECT_EQ(newOrderEvent->quantity_, 0);
}

TEST_F(EventParserTest, ParseNewOrder_LargeNumbers) {
    std::string csv = "D,user1212,999999,UBER,1000000,SELL,LIMIT,999999.99";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::NewOrder);
    
    auto newOrderEvent = dynamic_cast<NewOrderEvent*>(event.get());
    ASSERT_NE(newOrderEvent, nullptr);
    EXPECT_EQ(newOrderEvent->clientOrderId_, 999999);
    EXPECT_EQ(newOrderEvent->quantity_, 1000000);
    EXPECT_DOUBLE_EQ(newOrderEvent->price_, 999999.99);
}

TEST_F(EventParserTest, ParseInvalidEventType) {
    // Test invalid event types for all event categories
    EXPECT_THROW(parser->parse("X,user123,1001,AAPL,100,BUY,MARKET"), std::runtime_error);
    EXPECT_THROW(parser->parse("Y,user456,1002,MSFT,2001"), std::runtime_error);
    EXPECT_THROW(parser->parse("Z,user789,1003,GOOGL"), std::runtime_error);
    EXPECT_THROW(parser->parse("INVALID,user101"), std::runtime_error);
}

TEST_F(EventParserTest, ParseCancelOrder_Valid) {
    std::string csv = "F,user123,1001,AAPL,2001";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::CancelOrder);
    
    auto cancelOrderEvent = dynamic_cast<CancelOrderEvent*>(event.get());
    ASSERT_NE(cancelOrderEvent, nullptr);
    EXPECT_EQ(cancelOrderEvent->userId_, "user123");
    EXPECT_EQ(cancelOrderEvent->origOrderId_, 2001);
    EXPECT_EQ(cancelOrderEvent->symbol_, "AAPL");
}

TEST_F(EventParserTest, ParseCancelOrder_WithWhitespace) {
    std::string csv = " F , user456 , 1002 , MSFT , 2002 ";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::CancelOrder);
    
    auto cancelOrderEvent = dynamic_cast<CancelOrderEvent*>(event.get());
    ASSERT_NE(cancelOrderEvent, nullptr);
    EXPECT_EQ(cancelOrderEvent->userId_, "user456");
    EXPECT_EQ(cancelOrderEvent->origOrderId_, 2002);
    EXPECT_EQ(cancelOrderEvent->symbol_, "MSFT");
}



TEST_F(EventParserTest, ParseCancelOrder_TooFewTokens) {
    std::string csv = "F,user101,1004,TSLA";
    
    EXPECT_THROW(parser->parse(csv), std::runtime_error);
}

TEST_F(EventParserTest, ParseCancelOrder_InvalidOrigOrderId) {
    std::string csv = "F,user202,1005,TSLA,abc";
    
    EXPECT_THROW(parser->parse(csv), std::invalid_argument);
}

TEST_F(EventParserTest, ParseTopOfBook_Valid) {
    std::string csv = "V,user123,1001,AAPL";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::TopOfBook);
    
    auto topOfBookEvent = dynamic_cast<TopOfBookEvent*>(event.get());
    ASSERT_NE(topOfBookEvent, nullptr);
    EXPECT_EQ(topOfBookEvent->userId_, "user123");
    EXPECT_EQ(topOfBookEvent->symbol_, "AAPL");
}

TEST_F(EventParserTest, ParseTopOfBook_WithWhitespace) {
    std::string csv = " V , user456 , 1002 , MSFT ";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::TopOfBook);
    
    auto topOfBookEvent = dynamic_cast<TopOfBookEvent*>(event.get());
    ASSERT_NE(topOfBookEvent, nullptr);
    EXPECT_EQ(topOfBookEvent->userId_, "user456");
    EXPECT_EQ(topOfBookEvent->symbol_, "MSFT");
}



TEST_F(EventParserTest, ParseTopOfBook_TooFewTokens) {
    std::string csv = "V,user101,1003";
    
    EXPECT_THROW(parser->parse(csv), std::runtime_error);
}

TEST_F(EventParserTest, ParseQuit_Valid) {
    std::string csv = "Q";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::Quit);
    
    auto quitEvent = dynamic_cast<QuitEvent*>(event.get());
    ASSERT_NE(quitEvent, nullptr);
}

TEST_F(EventParserTest, ParseQuit_WithWhitespace) {
    std::string csv = " Quit ";
    
    auto event = parser->parse(csv);
    
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::Quit);
    
    auto quitEvent = dynamic_cast<QuitEvent*>(event.get());
    ASSERT_NE(quitEvent, nullptr);
}

TEST_F(EventParserTest, GetEventType_AllCases) {
    EXPECT_EQ(parser->getEventType("D,user123,1001,AAPL,100,BUY,MARKET"), EventType::NewOrder);
    EXPECT_EQ(parser->getEventType("f,user456,1002,MSFT,2001"), EventType::CancelOrder);
    EXPECT_EQ(parser->getEventType("v   ,user789,1003,GOOGL"), EventType::TopOfBook);
    EXPECT_EQ(parser->getEventType("q"), EventType::Quit);


    EXPECT_EQ(parser->getEventType("Z,user789,1003,GOOGL"), EventType::Invalid);
    EXPECT_EQ(parser->getEventType("D user789 1003 GOOGL"), EventType::Invalid);
    EXPECT_EQ(parser->getEventType("   "), EventType::Invalid);
    EXPECT_EQ(parser->getEventType("\t\r\n"), EventType::Invalid);
    EXPECT_EQ(parser->getEventType(""), EventType::Invalid);
}

// Test private template methods directly with different range types
TEST_F(EventParserTest, CreateNewOrderEvent_VariousRanges) {
   std::list<std::string> tokens = {"D", "user456", "1002", "MSFT", "50", "SELL", "LIMIT", "150.75"};
    
    auto verify = [](const auto& event) {
      ASSERT_NE(event, nullptr);
      EXPECT_EQ(event->userId_, "user456");
      EXPECT_EQ(event->clientOrderId_, 1002);
      EXPECT_EQ(event->symbol_, "MSFT");
      EXPECT_EQ(event->quantity_, 50);
      EXPECT_EQ(event->side_, Side::Sell);
      EXPECT_EQ(event->type_, Type::Limit);
      EXPECT_DOUBLE_EQ(event->price_, 150.75);
    };
    
    { // const lvalue
      const auto const_tokens = tokens;
      auto event = parser->createNewOrderEvent(const_tokens);
      verify(event);
    }

    { // rvalue
      auto event = parser->createNewOrderEvent(std::move(tokens));
      verify(event);
    }

    { // initializer list
      auto event = parser->createNewOrderEvent({"D", "user456", "1002", "MSFT", "50", "SELL", "LIMIT", "150.75"});
      verify(event);
    }
    
}

TEST_F(EventParserTest, CreateCancelOrderEvent_VariousRanges) {
  std::list<std::string> tokens {"F", "user123", "1001", "AAPL", "2001"};

    auto verify = [](const auto& event) {
      ASSERT_NE(event, nullptr);
      EXPECT_EQ(event->userId_, "user123");
      EXPECT_EQ(event->clientOrderId_, 1001);
      EXPECT_EQ(event->symbol_, "AAPL");
      EXPECT_EQ(event->origOrderId_, 2001);
    };

    {
      const auto const_tokens = tokens;
      auto event = parser->createCancelOrderEvent(const_tokens);
      verify(event);
    }

    {// test with rvalue
      auto event = parser->createCancelOrderEvent(std::move(tokens));
      verify(event);
    }

    {// initializer list
      auto event = parser->createCancelOrderEvent({"F", "user123", "1001", "AAPL", "2001"});
      verify(event);
    }
    
}


TEST_F(EventParserTest, CreateTopOfBookEvent_VariousRanges) {
  std::list<std::string> tokens {"V", "user456", "1002", "MSFT"};

  auto verify = [](const auto& event) {
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->userId_, "user456");
    EXPECT_EQ(event->clientOrderId_, 1002);
    EXPECT_EQ(event->symbol_, "MSFT");
  };

  {
    const auto const_tokens = tokens;
    auto event = parser->createTopOfBookEvent(const_tokens);
    verify(event);
  }

  {
    auto event = parser->createTopOfBookEvent(std::move(tokens));
    verify(event);
  }

  {
    auto event = parser->createTopOfBookEvent({"V", "user456", "1002", "MSFT"});
    verify(event);
  }
}

TEST_F(EventParserTest, CreateQuitEvent_VariousRanges) {
  std::list<std::string> tokens {"Q"};

  auto verify = [](const auto& event) {
    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->type(), EventType::Quit);
  };

  {
    const auto const_tokens = tokens;
    auto event = parser->createQuitEvent(const_tokens);
    verify(event);
  }


  {
    auto event = parser->createQuitEvent(std::move(tokens));
    verify(event);
  }


  {
    auto event = parser->createQuitEvent({"Q"});
    verify(event);
  }
}

} // namespace Exchange 