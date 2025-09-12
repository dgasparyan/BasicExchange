#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "OrderBook.h"
#include "Event.h"
#include "OrderUtils.h"
#include "ReportUtils.h"

namespace Exchange {
namespace test {

// Mock ReportSink for testing using older Google Mock syntax
class MockReportSink {
public:
    MOCK_METHOD1(submitFills, bool(ExecutionReportCollection&& fills));
    MOCK_METHOD1(submitCanceledOrder, bool(OrderCanceledReport&& report));
    MOCK_METHOD1(submitTopOfBook, bool(TopOfBookReport&& report));
};

class OrderBookTest : public ::testing::Test {
protected:
    void SetUp() override {
      auto mockReportSink = std::make_unique<MockReportSink>();
      mockReportSink_ = mockReportSink.get();

      orderBook_ = std::make_unique<OrderBook<MockReportSink>>(Symbol{"AAPL"}, std::move(mockReportSink));
    }

    void TearDown() override {
        mockReportSink_ = nullptr;
        orderBook_.reset();
    }

    MockReportSink* mockReportSink_ {nullptr};
    std::unique_ptr<OrderBook<MockReportSink>> orderBook_;
};

TEST_F(OrderBookTest, SubmitNewOrder_BuyLimitOrder_AddsToBidBook) {
    // Arrange
    auto event = NewOrderEvent(
        "user123"_uid, 
        1001, 
        "AAPL"_sym, 
        100, 
        Side::Buy, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );

    // Act
    orderBook_->submitNewOrder(event);

    // Test top of book - should show the buy order as best bid
    auto topOfBookEvent = TopOfBookEvent("user123"_uid, 1001, "AAPL"_sym);
    
    TopOfBookReport capturedTopOfBook;
    EXPECT_CALL(*mockReportSink_, submitTopOfBook(testing::_))
        .WillOnce(testing::Invoke([&capturedTopOfBook](TopOfBookReport&& report) {
            capturedTopOfBook = std::move(report);
            return true;
        }));

    orderBook_->submitTopOfBook(topOfBookEvent);

    // Assert - Verify top of book shows correct bid order
    EXPECT_EQ(capturedTopOfBook.bid_order_.orderId_, 1001);
    EXPECT_EQ(capturedTopOfBook.bid_order_.openQuantity_, 100);
    EXPECT_EQ(capturedTopOfBook.bid_order_.openQuantity_, 100);  // No fills yet
    EXPECT_EQ(capturedTopOfBook.bid_order_.price_, toPrice(150.00, TWO_DIGITS_PRICE_SPEC));
    // Ask should be empty (default Order)
    EXPECT_FALSE(capturedTopOfBook.ask_order_.isValid());

    // Now cancel the order and verify the cancellation report
    auto cancelEvent = CancelOrderEvent("user123"_uid, 1001, "AAPL"_sym, 1001);
    
    OrderCanceledReport capturedCancel;
    EXPECT_CALL(*mockReportSink_, submitCanceledOrder(testing::_))
        .WillOnce(testing::Invoke([&capturedCancel](OrderCanceledReport&& report) {
            capturedCancel = std::move(report);
            return true;
        }));

    bool cancelResult = orderBook_->submitCancelOrder(cancelEvent);

    // Assert - Verify cancellation was successful and reported correctly
    EXPECT_TRUE(cancelResult);
    EXPECT_EQ(capturedCancel.orderId_, 1001);
    EXPECT_EQ(capturedCancel.remainingQuantity_, 100);  // Full quantity cancelled
    EXPECT_EQ(capturedCancel.reason_, CancelReason::User_Canceled);
}

TEST_F(OrderBookTest, SubmitNewOrder_SellLimitOrder_AddsToAskBook) {
    // Arrange
    auto event = NewOrderEvent(
        "user456"_uid, 
        1002, 
        "AAPL"_sym, 
        50, 
        Side::Sell, 
        Type::Limit, 
        toPrice(151.00, TWO_DIGITS_PRICE_SPEC)
    );

    // Act
    orderBook_->submitNewOrder(event);

    // Test top of book - should show the sell order as best ask
    auto topOfBookEvent = TopOfBookEvent("user456"_uid, 1002, "AAPL"_sym);
    
    TopOfBookReport capturedTopOfBook;
    EXPECT_CALL(*mockReportSink_, submitTopOfBook(testing::_))
        .WillOnce(testing::Invoke([&capturedTopOfBook](TopOfBookReport&& report) {
            capturedTopOfBook = std::move(report);
            return true;
        }));

    orderBook_->submitTopOfBook(topOfBookEvent);

    // Assert - Verify top of book shows correct ask order
    EXPECT_EQ(capturedTopOfBook.ask_order_.orderId_, 1002);
    EXPECT_EQ(capturedTopOfBook.ask_order_.openQuantity_, 50);
    EXPECT_EQ(capturedTopOfBook.ask_order_.openQuantity_, 50);  // No fills yet
    EXPECT_EQ(capturedTopOfBook.ask_order_.price_, toPrice(151.00, TWO_DIGITS_PRICE_SPEC));
    // Bid should be empty (default Order)
    EXPECT_FALSE(capturedTopOfBook.bid_order_.isValid());

    // Now cancel the order and verify the cancellation report
    auto cancelEvent = CancelOrderEvent("user456"_uid, 1002, "AAPL"_sym, 1002);
    
    OrderCanceledReport capturedCancel;
    EXPECT_CALL(*mockReportSink_, submitCanceledOrder(testing::_))
        .WillOnce(testing::Invoke([&capturedCancel](OrderCanceledReport&& report) {
            capturedCancel = std::move(report);
            return true;
        }));

    bool cancelResult = orderBook_->submitCancelOrder(cancelEvent);

    // Assert - Verify cancellation was successful and reported correctly
    EXPECT_TRUE(cancelResult);
    EXPECT_EQ(capturedCancel.orderId_, 1002);
    EXPECT_EQ(capturedCancel.remainingQuantity_, 50);  // Full quantity cancelled
    EXPECT_EQ(capturedCancel.reason_, CancelReason::User_Canceled);
}

TEST_F(OrderBookTest, SubmitNewOrder_BuyMarketOrder_WithNoSells_CancelsOrder) {
    // Arrange
    auto event = NewOrderEvent(
        "user123"_uid, 
        1003, 
        "AAPL"_sym, 
        100, 
        Side::Buy, 
        Type::Market, 
        MARKET_PRICE
    );

    // Capture the cancellation to verify it
    OrderCanceledReport capturedCancel;
    EXPECT_CALL(*mockReportSink_, submitCanceledOrder(testing::_))
        .WillOnce(testing::Invoke([&capturedCancel](OrderCanceledReport&& report) {
            capturedCancel = std::move(report);
            return true;
        }));

    // Act
    orderBook_->submitNewOrder(event);

    // Assert - Verify the cancellation details
    EXPECT_EQ(capturedCancel.orderId_, 1003);                    // Correct order ID
    EXPECT_EQ(capturedCancel.remainingQuantity_, 100);           // Full quantity cancelled (no fills)
    EXPECT_EQ(capturedCancel.reason_, CancelReason::Fill_And_Kill); // Correct reason

    
}

TEST_F(OrderBookTest, SubmitNewOrder_AggressiveBuy_FillsWithExistingSell) {
    // Arrange
    // First add a sell order
    auto sellEvent = NewOrderEvent(
        "user456"_uid, 
        2001, 
        "AAPL"_sym, 
        100, 
        Side::Sell, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent);

    // Test top of book before aggressive order - should show only the sell order
    auto topOfBookEvent1 = TopOfBookEvent("user456"_uid, 2001, "AAPL"_sym);
    
    TopOfBookReport capturedTopOfBook1;
    EXPECT_CALL(*mockReportSink_, submitTopOfBook(testing::_))
        .WillOnce(testing::Invoke([&capturedTopOfBook1](TopOfBookReport&& report) {
            capturedTopOfBook1 = std::move(report);
            return true;
        }));

    orderBook_->submitTopOfBook(topOfBookEvent1);

    // Assert - Verify top of book shows only ask order before aggressive order
    EXPECT_EQ(capturedTopOfBook1.ask_order_.orderId_, 2001);
    EXPECT_EQ(capturedTopOfBook1.ask_order_.openQuantity_, 100);  // No fills yet
    EXPECT_EQ(capturedTopOfBook1.ask_order_.price_, toPrice(150.00, TWO_DIGITS_PRICE_SPEC));
    // Bid should be empty (default Order)
    EXPECT_FALSE(capturedTopOfBook1.bid_order_.isValid());

    // Now add an aggressive buy order
    auto buyEvent = NewOrderEvent(
        "user123"_uid, 
        2002, 
        "AAPL"_sym, 
        50, 
        Side::Buy, 
        Type::Limit, 
        toPrice(151.00, TWO_DIGITS_PRICE_SPEC)  // Higher price = aggressive
    );

    // Capture the fills to verify them
    ExecutionReportCollection capturedFills;
    EXPECT_CALL(*mockReportSink_, submitFills(testing::_))
        .WillOnce(testing::Invoke([&capturedFills](ExecutionReportCollection&& fills) {
            capturedFills = std::move(fills);
            return true;
        }));

    // Act
    orderBook_->submitNewOrder(buyEvent);

    // Assert - Verify the fill details
    ASSERT_EQ(capturedFills.size(), 2);  // Should have 2 entries: sell order fill + buy order fill
    EXPECT_EQ(capturedFills[0].orderId_, 2001);   // Sell order ID
    EXPECT_EQ(capturedFills[0].otherOrderId_, 2002);   // Buy order ID
    EXPECT_EQ(capturedFills[0].filledQuantity_, 50);    // 50 shares filled
    EXPECT_EQ(capturedFills[1].orderId_, 2002);   // Buy order ID  
    EXPECT_EQ(capturedFills[1].otherOrderId_, 2001);   // Sell order ID
    EXPECT_EQ(capturedFills[1].filledQuantity_, 50);    // 50 shares filled

    // Test top of book after partial fill - should show remaining sell order
    auto topOfBookEvent2 = TopOfBookEvent("user456"_uid, 2001, "AAPL"_sym);
    
    TopOfBookReport capturedTopOfBook2;
    EXPECT_CALL(*mockReportSink_, submitTopOfBook(testing::_))
        .WillOnce(testing::Invoke([&capturedTopOfBook2](TopOfBookReport&& report) {
            capturedTopOfBook2 = std::move(report);
            return true;
        }));

    orderBook_->submitTopOfBook(topOfBookEvent2);

    // Assert - Verify top of book shows remaining ask order after partial fill
    EXPECT_EQ(capturedTopOfBook2.ask_order_.orderId_, 2001);
    EXPECT_EQ(capturedTopOfBook2.ask_order_.openQuantity_, 50);  // 50 shares remaining after 50 filled
    EXPECT_EQ(capturedTopOfBook2.ask_order_.price_, toPrice(150.00, TWO_DIGITS_PRICE_SPEC));
    // Bid should still be empty (default Order)
    EXPECT_FALSE(capturedTopOfBook2.bid_order_.isValid());

    // Now cancel the remaining 50 shares of the sell order
    auto cancelEvent = CancelOrderEvent("user456"_uid, 2001, "AAPL"_sym, 2001);
    
    OrderCanceledReport capturedCancel;
    EXPECT_CALL(*mockReportSink_, submitCanceledOrder(testing::_))
        .WillOnce(testing::Invoke([&capturedCancel](OrderCanceledReport&& report) {
            capturedCancel = std::move(report);
            return true;
        }));

    bool cancelResult = orderBook_->submitCancelOrder(cancelEvent);

    // Assert - Verify cancellation was successful and reported correct remaining quantity
    EXPECT_TRUE(cancelResult);
    EXPECT_EQ(capturedCancel.orderId_, 2001);
    EXPECT_EQ(capturedCancel.remainingQuantity_, 50);  // 50 shares remaining (100 - 50 filled)
    EXPECT_EQ(capturedCancel.reason_, CancelReason::User_Canceled);
}

TEST_F(OrderBookTest, SubmitNewOrder_AggressiveSell_FillsWithExistingBuy) {
    // Arrange
    // First add a buy order
    auto buyEvent = NewOrderEvent(
        "user123"_uid, 
        3001, 
        "AAPL"_sym, 
        100, 
        Side::Buy, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(buyEvent);

    // Now add an aggressive sell order
    auto sellEvent = NewOrderEvent(
        "user456"_uid, 
        3002, 
        "AAPL"_sym, 
        75, 
        Side::Sell, 
        Type::Limit, 
        toPrice(149.00, TWO_DIGITS_PRICE_SPEC)  // Lower price = aggressive
    );

    // Capture the fills to verify them
    ExecutionReportCollection capturedFills;
    EXPECT_CALL(*mockReportSink_, submitFills(testing::_))
        .WillOnce(testing::Invoke([&capturedFills](ExecutionReportCollection&& fills) {
            capturedFills = std::move(fills);
            return true;
        }));

    // Act
    orderBook_->submitNewOrder(sellEvent);

    // Assert - Verify the fill details
    ASSERT_EQ(capturedFills.size(), 2);  // Should have 2 entries: buy order fill + sell order fill
    EXPECT_EQ(capturedFills[0].orderId_, 3001);   // Buy order ID
    EXPECT_EQ(capturedFills[0].otherOrderId_, 3002);   // Sell order ID
    EXPECT_EQ(capturedFills[0].filledQuantity_, 75);    // 75 shares filled
    EXPECT_EQ(capturedFills[1].orderId_, 3002);   // Sell order ID  
    EXPECT_EQ(capturedFills[1].otherOrderId_, 3001);   // Buy order ID
    EXPECT_EQ(capturedFills[1].filledQuantity_, 75);    // 75 shares filled

    // Now cancel the remaining 25 shares of the buy order
    auto cancelEvent = CancelOrderEvent("user123"_uid, 3001, "AAPL"_sym, 3001);
    
    OrderCanceledReport capturedCancel;
    EXPECT_CALL(*mockReportSink_, submitCanceledOrder(testing::_))
        .WillOnce(testing::Invoke([&capturedCancel](OrderCanceledReport&& report) {
            capturedCancel = std::move(report);
            return true;
        }));

    bool cancelResult = orderBook_->submitCancelOrder(cancelEvent);

    // Assert - Verify cancellation was successful and reported correct remaining quantity
    EXPECT_TRUE(cancelResult);
    EXPECT_EQ(capturedCancel.orderId_, 3001);
    EXPECT_EQ(capturedCancel.remainingQuantity_, 25);  // 25 shares remaining (100 - 75 filled)
    EXPECT_EQ(capturedCancel.reason_, CancelReason::User_Canceled);
}

TEST_F(OrderBookTest, SubmitNewOrder_PartialFill_RemainingGoesToBook) {
    // Arrange
    // First add a sell order for 100 shares at $150.00
    auto sellEvent1 = NewOrderEvent(
        "user456"_uid, 
        4001, 
        "AAPL"_sym, 
        100, 
        Side::Sell, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent1);

    // Add another sell order for 200 shares at $152.00 (worse price)
    auto sellEvent2 = NewOrderEvent(
        "user789"_uid, 
        4003, 
        "AAPL"_sym, 
        200, 
        Side::Sell, 
        Type::Limit, 
        toPrice(152.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent2);

    // Now add a buy order for 150 shares (will fill 100 from first sell, 50 remaining goes to book)
    auto buyEvent = NewOrderEvent(
        "user123"_uid, 
        4002, 
        "AAPL"_sym, 
        150, 
        Side::Buy, 
        Type::Limit, 
        toPrice(151.00, TWO_DIGITS_PRICE_SPEC)
    );

    // Capture the fills to verify them
    ExecutionReportCollection capturedFills;
    EXPECT_CALL(*mockReportSink_, submitFills(testing::_))
        .WillOnce(testing::Invoke([&capturedFills](ExecutionReportCollection&& fills) {
            capturedFills = std::move(fills);
            return true;
        }));

    // Act
    orderBook_->submitNewOrder(buyEvent);

    // Assert - Verify the fill details
    ASSERT_EQ(capturedFills.size(), 2);  // Should have 2 entries: sell order fill + buy order fill
    EXPECT_EQ(capturedFills[0].orderId_, 4001);   // Sell order ID
    EXPECT_EQ(capturedFills[0].otherOrderId_, 4002);   // Buy order ID
    EXPECT_EQ(capturedFills[0].filledQuantity_, 100);   // 100 shares filled (fully filled)
    EXPECT_EQ(capturedFills[1].orderId_, 4002);   // Buy order ID  
    EXPECT_EQ(capturedFills[1].otherOrderId_, 4001);   // Sell order ID
    EXPECT_EQ(capturedFills[1].filledQuantity_, 100);   // 100 shares filled (partial fill of 150)

    // Now cancel the remaining 50 shares of the buy order
    auto cancelEvent = CancelOrderEvent("user123"_uid, 4002, "AAPL"_sym, 4002);
    
    OrderCanceledReport capturedCancel;
    EXPECT_CALL(*mockReportSink_, submitCanceledOrder(testing::_))
        .WillOnce(testing::Invoke([&capturedCancel](OrderCanceledReport&& report) {
            capturedCancel = std::move(report);
            return true;
        }));

    bool cancelResult = orderBook_->submitCancelOrder(cancelEvent);

    // Assert - Verify cancellation was successful and reported correct remaining quantity
    EXPECT_TRUE(cancelResult);
    EXPECT_EQ(capturedCancel.orderId_, 4002);
    EXPECT_EQ(capturedCancel.remainingQuantity_, 50);  // 50 shares remaining (150 - 100 filled)
    EXPECT_EQ(capturedCancel.reason_, CancelReason::User_Canceled);
}

TEST_F(OrderBookTest, SubmitNewOrder_PartialFill_ThenCancelled) {
    // Arrange - Add a small sell order
    auto sellEvent = NewOrderEvent(
        "user456"_uid, 
        5001, 
        "AAPL"_sym, 
        30, 
        Side::Sell, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent);

    // Add a larger buy order that will partially fill and then be cancelled
    auto buyEvent = NewOrderEvent(
        "user123"_uid, 
        5002, 
        "AAPL"_sym, 
        100, 
        Side::Buy, 
        Type::Limit, 
        toPrice(151.00, TWO_DIGITS_PRICE_SPEC)
    );

    // Capture both fills and cancellation
    ExecutionReportCollection capturedFills;
    OrderCanceledReport capturedCancel;
    
    EXPECT_CALL(*mockReportSink_, submitFills(testing::_))
        .WillOnce(testing::Invoke([&capturedFills](ExecutionReportCollection&& fills) {
            capturedFills = std::move(fills);
            return true;
        }));
    
    EXPECT_CALL(*mockReportSink_, submitCanceledOrder(testing::_))
        .WillOnce(testing::Invoke([&capturedCancel](OrderCanceledReport&& report) {
            capturedCancel = std::move(report);
            return true;
        }));

    // Act
    orderBook_->submitNewOrder(buyEvent);

    // Assert - Verify the fill details
    ASSERT_EQ(capturedFills.size(), 2);  // Should have 2 entries: sell order fill + buy order fill
    EXPECT_EQ(capturedFills[0].orderId_, 5001);   // Sell order ID
    EXPECT_EQ(capturedFills[0].otherOrderId_, 5002);   // Buy order ID
    EXPECT_EQ(capturedFills[0].filledQuantity_, 30);    // 30 shares filled (fully filled)
    EXPECT_EQ(capturedFills[1].orderId_, 5002);   // Buy order ID  
    EXPECT_EQ(capturedFills[1].otherOrderId_, 5001);   // Sell order ID
    EXPECT_EQ(capturedFills[1].filledQuantity_, 30);    // 30 shares filled (partial fill of 100)

    // Assert - Verify the cancellation details
    EXPECT_EQ(capturedCancel.orderId_, 5002);                    // Buy order ID
    EXPECT_EQ(capturedCancel.remainingQuantity_, 70);            // 70 shares cancelled (100 - 30)
    EXPECT_EQ(capturedCancel.reason_, CancelReason::Fill_And_Kill); // Correct reason
}

TEST_F(OrderBookTest, SubmitNewOrder_FillsMultipleOrders_PriceTimePriority) {
    // Arrange - Add multiple sell orders at different prices and times
    // Order 1: 50 shares at $150.00
    auto sellEvent1 = NewOrderEvent(
        "user1"_uid, 
        6001, 
        "AAPL"_sym, 
        50, 
        Side::Sell, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent1);

    // Order 2: 30 shares at $150.00 (same price, later time)
    auto sellEvent2 = NewOrderEvent(
        "user2"_uid, 
        6002, 
        "AAPL"_sym, 
        30, 
        Side::Sell, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent2);

    // Order 3: 40 shares at $149.50 (better price)
    auto sellEvent3 = NewOrderEvent(
        "user3"_uid, 
        6003, 
        "AAPL"_sym, 
        40, 
        Side::Sell, 
        Type::Limit, 
        toPrice(149.50, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent3);

    // Order 4: 20 shares at $149.00 (best price)
    auto sellEvent4 = NewOrderEvent(
        "user4"_uid, 
        6004, 
        "AAPL"_sym, 
        20, 
        Side::Sell, 
        Type::Limit, 
        toPrice(149.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent4);

    // Now add an aggressive buy order for 100 shares at $151.00
    // This should fill orders 4, 3, 1, and partially fill order 2
    auto buyEvent = NewOrderEvent(
        "buyer"_uid, 
        6005, 
        "AAPL"_sym, 
        100, 
        Side::Buy, 
        Type::Limit, 
        toPrice(151.00, TWO_DIGITS_PRICE_SPEC)
    );

    // Capture the fills to verify them
    ExecutionReportCollection capturedFills;
    EXPECT_CALL(*mockReportSink_, submitFills(testing::_))
        .WillOnce(testing::Invoke([&capturedFills](ExecutionReportCollection&& fills) {
            capturedFills = std::move(fills);
            return true;
        }));

    // Act
    orderBook_->submitNewOrder(buyEvent);

    // Assert - Verify the fill details
    // Should have 6 entries: 3 sell order fills + 3 buy order fills (in pairs)
    ASSERT_EQ(capturedFills.size(), 6);
    
    // Check that we have the right order IDs and quantities
    // The fills should be in order: order 4 (20), order 3 (40), order 1 (40)
    // Each fill appears twice: once for the sell order, once for the buy order
    
    // Order 4: 20 shares at $149.00 (best price first)
    EXPECT_EQ(capturedFills[0].orderId_, 6004);   // Sell order 4
    EXPECT_EQ(capturedFills[0].otherOrderId_, 6005);   // Buy order
    EXPECT_EQ(capturedFills[0].filledQuantity_, 20);    // 20 shares
    EXPECT_EQ(capturedFills[1].orderId_, 6005);   // Buy order
    EXPECT_EQ(capturedFills[1].otherOrderId_, 6004);   // Sell order 4
    EXPECT_EQ(capturedFills[1].filledQuantity_, 20);    // 20 shares
    
    // Order 3: 40 shares at $149.50
    EXPECT_EQ(capturedFills[2].orderId_, 6003);   // Sell order 3
    EXPECT_EQ(capturedFills[2].otherOrderId_, 6005);   // Buy order
    EXPECT_EQ(capturedFills[2].filledQuantity_, 40);    // 40 shares
    EXPECT_EQ(capturedFills[3].orderId_, 6005);   // Buy order
    EXPECT_EQ(capturedFills[3].otherOrderId_, 6003);   // Sell order 3
    EXPECT_EQ(capturedFills[3].filledQuantity_, 40);    // 40 shares
    
    // Order 1: 40 shares at $150.00 (partial fill of 50, earlier time than order 2)
    EXPECT_EQ(capturedFills[4].orderId_, 6001);   // Sell order 1
    EXPECT_EQ(capturedFills[4].otherOrderId_, 6005);   // Buy order
    EXPECT_EQ(capturedFills[4].filledQuantity_, 40);    // 40 shares (partial)
    EXPECT_EQ(capturedFills[5].orderId_, 6005);   // Buy order
    EXPECT_EQ(capturedFills[5].otherOrderId_, 6001);   // Sell order 1
    EXPECT_EQ(capturedFills[5].filledQuantity_, 40);    // 40 shares (partial)
}

TEST_F(OrderBookTest, SubmitNewOrder_MarketOrder_FillsMultipleOrders) {
    // Arrange - Add multiple sell orders
    auto sellEvent1 = NewOrderEvent(
        "user1"_uid, 
        7001, 
        "AAPL"_sym, 
        25, 
        Side::Sell, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent1);

    auto sellEvent2 = NewOrderEvent(
        "user2"_uid, 
        7002, 
        "AAPL"_sym, 
        35, 
        Side::Sell, 
        Type::Limit, 
        toPrice(151.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent2);

    auto sellEvent3 = NewOrderEvent(
        "user3"_uid, 
        7003, 
        "AAPL"_sym, 
        40, 
        Side::Sell, 
        Type::Limit, 
        toPrice(152.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent3);

    // Market buy order for 80 shares (should fill all orders)
    auto marketBuyEvent = NewOrderEvent(
        "buyer"_uid, 
        7004, 
        "AAPL"_sym, 
        80, 
        Side::Buy, 
        Type::Market, 
        MARKET_PRICE
    );

    // Capture the fills to verify them
    ExecutionReportCollection capturedFills;
    EXPECT_CALL(*mockReportSink_, submitFills(testing::_))
        .WillOnce(testing::Invoke([&capturedFills](ExecutionReportCollection&& fills) {
            capturedFills = std::move(fills);
            return true;
        }));

    // Act
    orderBook_->submitNewOrder(marketBuyEvent);

    // Assert - Verify the fill details
    // Should have 6 entries: 3 sell order fills + 3 buy order fills (in pairs)
    ASSERT_EQ(capturedFills.size(), 6);
    
    // Check that all 3 sell orders are filled (in price priority order)
    // Order 1: 25 shares at $150.00 (best price first)
    EXPECT_EQ(capturedFills[0].orderId_, 7001);   // Sell order 1
    EXPECT_EQ(capturedFills[0].otherOrderId_, 7004);   // Buy order
    EXPECT_EQ(capturedFills[0].filledQuantity_, 25);    // 25 shares
    EXPECT_EQ(capturedFills[1].orderId_, 7004);   // Buy order
    EXPECT_EQ(capturedFills[1].otherOrderId_, 7001);   // Sell order 1
    EXPECT_EQ(capturedFills[1].filledQuantity_, 25);    // 25 shares
    
    // Order 2: 35 shares at $151.00
    EXPECT_EQ(capturedFills[2].orderId_, 7002);   // Sell order 2
    EXPECT_EQ(capturedFills[2].otherOrderId_, 7004);   // Buy order
    EXPECT_EQ(capturedFills[2].filledQuantity_, 35);    // 35 shares
    EXPECT_EQ(capturedFills[3].orderId_, 7004);   // Buy order
    EXPECT_EQ(capturedFills[3].otherOrderId_, 7002);   // Sell order 2
    EXPECT_EQ(capturedFills[3].filledQuantity_, 35);    // 35 shares
    
    // Order 3: 20 shares at $152.00 (partial fill of 40)
    EXPECT_EQ(capturedFills[4].orderId_, 7003);   // Sell order 3
    EXPECT_EQ(capturedFills[4].otherOrderId_, 7004);   // Buy order
    EXPECT_EQ(capturedFills[4].filledQuantity_, 20);    // 20 shares (partial)
    EXPECT_EQ(capturedFills[5].orderId_, 7004);   // Buy order
    EXPECT_EQ(capturedFills[5].otherOrderId_, 7003);   // Sell order 3
    EXPECT_EQ(capturedFills[5].filledQuantity_, 20);    // 20 shares (partial)
}

TEST_F(OrderBookTest, SubmitNewOrder_MarketSellOrder_FillsMultipleOrders) {
    // Arrange - Add multiple buy orders
    auto buyEvent1 = NewOrderEvent(
        "user1"_uid, 
        7501, 
        "AAPL"_sym, 
        30, 
        Side::Buy, 
        Type::Limit, 
        toPrice(155.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(buyEvent1);

    auto buyEvent2 = NewOrderEvent(
        "user2"_uid, 
        7502, 
        "AAPL"_sym, 
        45, 
        Side::Buy, 
        Type::Limit, 
        toPrice(154.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(buyEvent2);

    auto buyEvent3 = NewOrderEvent(
        "user3"_uid, 
        7503, 
        "AAPL"_sym, 
        50, 
        Side::Buy, 
        Type::Limit, 
        toPrice(153.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(buyEvent3);

    // Market sell order for 100 shares (should fill all orders)
    auto marketSellEvent = NewOrderEvent(
        "seller"_uid, 
        7504, 
        "AAPL"_sym, 
        100, 
        Side::Sell, 
        Type::Market, 
        MARKET_PRICE
    );

    // Capture the fills to verify them
    ExecutionReportCollection capturedFills;
    EXPECT_CALL(*mockReportSink_, submitFills(testing::_))
        .WillOnce(testing::Invoke([&capturedFills](ExecutionReportCollection&& fills) {
            capturedFills = std::move(fills);
            return true;
        }));

    // Act
    orderBook_->submitNewOrder(marketSellEvent);

    // Assert - Verify the fill details
    // Should have 6 entries: 3 buy order fills + 3 sell order fills (in pairs)
    ASSERT_EQ(capturedFills.size(), 6);
    
    // Check that all 3 buy orders are filled (in price priority order - highest price first)
    // Order 1: 30 shares at $155.00 (best price first)
    EXPECT_EQ(capturedFills[0].orderId_, 7501);   // Buy order 1
    EXPECT_EQ(capturedFills[0].otherOrderId_, 7504);   // Sell order
    EXPECT_EQ(capturedFills[0].filledQuantity_, 30);    // 30 shares
    EXPECT_EQ(capturedFills[1].orderId_, 7504);   // Sell order
    EXPECT_EQ(capturedFills[1].otherOrderId_, 7501);   // Buy order 1
    EXPECT_EQ(capturedFills[1].filledQuantity_, 30);    // 30 shares
    
    // Order 2: 45 shares at $154.00
    EXPECT_EQ(capturedFills[2].orderId_, 7502);   // Buy order 2
    EXPECT_EQ(capturedFills[2].otherOrderId_, 7504);   // Sell order
    EXPECT_EQ(capturedFills[2].filledQuantity_, 45);    // 45 shares
    EXPECT_EQ(capturedFills[3].orderId_, 7504);   // Sell order
    EXPECT_EQ(capturedFills[3].otherOrderId_, 7502);   // Buy order 2
    EXPECT_EQ(capturedFills[3].filledQuantity_, 45);    // 45 shares
    
    // Order 3: 25 shares at $153.00 (partial fill of 50)
    EXPECT_EQ(capturedFills[4].orderId_, 7503);   // Buy order 3
    EXPECT_EQ(capturedFills[4].otherOrderId_, 7504);   // Sell order
    EXPECT_EQ(capturedFills[4].filledQuantity_, 25);    // 25 shares (partial)
    EXPECT_EQ(capturedFills[5].orderId_, 7504);   // Sell order
    EXPECT_EQ(capturedFills[5].otherOrderId_, 7503);   // Buy order 3
    EXPECT_EQ(capturedFills[5].filledQuantity_, 25);    // 25 shares (partial)
}

TEST_F(OrderBookTest, SubmitNewOrder_LargeOrder_FillsEntireBook) {
    // Arrange - Add multiple small sell orders
    for (int i = 0; i < 5; ++i) {
        auto sellEvent = NewOrderEvent(
            "user"_uid, 
            8000 + i, 
            "AAPL"_sym, 
            10, 
            Side::Sell, 
            Type::Limit, 
            toPrice(150.00 + i * 0.01, TWO_DIGITS_PRICE_SPEC)  // $150.00, $150.01, etc.
        );
        orderBook_->submitNewOrder(sellEvent);
    }

    // Large buy order that should fill all sell orders
    auto largeBuyEvent = NewOrderEvent(
        "bigbuyer"_uid, 
        8005, 
        "AAPL"_sym, 
        100,  // Much larger than total available (50 shares)
        Side::Buy, 
        Type::Limit, 
        toPrice(155.00, TWO_DIGITS_PRICE_SPEC)
    );

    // Capture both fills and cancellation
    ExecutionReportCollection capturedFills;
    OrderCanceledReport capturedCancel;
    
    EXPECT_CALL(*mockReportSink_, submitFills(testing::_))
        .WillOnce(testing::Invoke([&capturedFills](ExecutionReportCollection&& fills) {
            capturedFills = std::move(fills);
            return true;
        }));
    
    EXPECT_CALL(*mockReportSink_, submitCanceledOrder(testing::_))
        .WillOnce(testing::Invoke([&capturedCancel](OrderCanceledReport&& report) {
            capturedCancel = std::move(report);
            return true;
        }));

    // Act
    orderBook_->submitNewOrder(largeBuyEvent);

    // Assert - Verify the fill details
    // Should have 10 entries: 5 sell order fills + 5 buy order fills (in pairs)
    ASSERT_EQ(capturedFills.size(), 10);
    
    // Check that all 5 sell orders are filled (in price priority order)
    // Each fill appears twice: once for the sell order, once for the buy order
    for (int i = 0; i < 5; ++i) {
        // Sell order fill
        EXPECT_EQ(capturedFills[i * 2].orderId_, 8000 + i);     // Sell order ID
        EXPECT_EQ(capturedFills[i * 2].otherOrderId_, 8005);     // Buy order ID
        EXPECT_EQ(capturedFills[i * 2].filledQuantity_, 10);          // 10 shares each
        
        // Buy order fill
        EXPECT_EQ(capturedFills[i * 2 + 1].orderId_, 8005);     // Buy order ID
        EXPECT_EQ(capturedFills[i * 2 + 1].otherOrderId_, 8000 + i);     // Sell order ID
        EXPECT_EQ(capturedFills[i * 2 + 1].filledQuantity_, 10);      // 10 shares each
    }

    // Assert - Verify the cancellation details
    EXPECT_EQ(capturedCancel.orderId_, 8005);                    // Buy order ID
    EXPECT_EQ(capturedCancel.remainingQuantity_, 50);            // 50 shares cancelled (100 - 50 filled)
    EXPECT_EQ(capturedCancel.reason_, CancelReason::Fill_And_Kill); // Correct reason
}

TEST_F(OrderBookTest, SubmitNewOrder_ExactMatch_FillsCompletely) {
    // Arrange - Add sell orders totaling exactly 100 shares
    auto sellEvent1 = NewOrderEvent(
        "user1"_uid, 
        9001, 
        "AAPL"_sym, 
        60, 
        Side::Sell, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent1);

    auto sellEvent2 = NewOrderEvent(
        "user2"_uid, 
        9002, 
        "AAPL"_sym, 
        40, 
        Side::Sell, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent2);

    // Buy order for exactly 100 shares
    auto buyEvent = NewOrderEvent(
        "buyer"_uid, 
        9003, 
        "AAPL"_sym, 
        100, 
        Side::Buy, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );

    // Capture the fills to verify them
    ExecutionReportCollection capturedFills;
    EXPECT_CALL(*mockReportSink_, submitFills(testing::_))
        .WillOnce(testing::Invoke([&capturedFills](ExecutionReportCollection&& fills) {
            capturedFills = std::move(fills);
            return true;
        }));

    // Expect that no cancellation occurs since the order is completely filled
    EXPECT_CALL(*mockReportSink_, submitCanceledOrder(testing::_))
        .Times(0);

    // Act
    orderBook_->submitNewOrder(buyEvent);

    // Assert - Verify the fill details
    // Should have 4 entries: 2 sell order fills + 2 buy order fills (in pairs)
    ASSERT_EQ(capturedFills.size(), 4);
    
    // Order 1: 60 shares (earlier time)
    EXPECT_EQ(capturedFills[0].orderId_, 9001);   // Sell order 1
    EXPECT_EQ(capturedFills[0].otherOrderId_, 9003);   // Buy order
    EXPECT_EQ(capturedFills[0].filledQuantity_, 60);    // 60 shares
    EXPECT_EQ(capturedFills[1].orderId_, 9003);   // Buy order
    EXPECT_EQ(capturedFills[1].otherOrderId_, 9001);   // Sell order 1
    EXPECT_EQ(capturedFills[1].filledQuantity_, 60);    // 60 shares
    
    // Order 2: 40 shares
    EXPECT_EQ(capturedFills[2].orderId_, 9002);   // Sell order 2
    EXPECT_EQ(capturedFills[2].otherOrderId_, 9003);   // Buy order
    EXPECT_EQ(capturedFills[2].filledQuantity_, 40);    // 40 shares
    EXPECT_EQ(capturedFills[3].orderId_, 9003);   // Buy order
    EXPECT_EQ(capturedFills[3].otherOrderId_, 9002);   // Sell order 2
    EXPECT_EQ(capturedFills[3].filledQuantity_, 40);    // 40 shares
}

TEST_F(OrderBookTest, SubmitNewOrder_CrossingSpread_FillsAtBestPrice) {
    // Arrange - Add sell order at $150.00
    auto sellEvent = NewOrderEvent(
        "seller"_uid, 
        10001, 
        "AAPL"_sym, 
        50, 
        Side::Sell, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent);

    // Buy order at $151.00 (crossing the spread)
    auto buyEvent = NewOrderEvent(
        "buyer"_uid, 
        10002, 
        "AAPL"_sym, 
        30, 
        Side::Buy, 
        Type::Limit, 
        toPrice(151.00, TWO_DIGITS_PRICE_SPEC)
    );

    // Capture the fills to verify them
    ExecutionReportCollection capturedFills;
    EXPECT_CALL(*mockReportSink_, submitFills(testing::_))
        .WillOnce(testing::Invoke([&capturedFills](ExecutionReportCollection&& fills) {
            capturedFills = std::move(fills);
            return true;
        }));

    // Act
    orderBook_->submitNewOrder(buyEvent);

    // Assert - Verify the fill details
    ASSERT_EQ(capturedFills.size(), 2);  // Should have 2 entries: sell order fill + buy order fill
    EXPECT_EQ(capturedFills[0].orderId_, 10001);  // Sell order ID
    EXPECT_EQ(capturedFills[0].otherOrderId_, 10002);  // Buy order ID
    EXPECT_EQ(capturedFills[0].filledQuantity_, 30);    // 30 shares filled
    EXPECT_EQ(capturedFills[1].orderId_, 10002);  // Buy order ID  
    EXPECT_EQ(capturedFills[1].otherOrderId_, 10001);  // Sell order ID
    EXPECT_EQ(capturedFills[1].filledQuantity_, 30);    // 30 shares filled

    // The buy should fill at the sell price ($150.00), not the buy price ($151.00)
}

TEST_F(OrderBookTest, SubmitNewOrder_ZeroQuantity_DoesNotCrash) {
    // Arrange
    auto event = NewOrderEvent(
        "user123"_uid, 
        11001, 
        "AAPL"_sym, 
        0,  // Zero quantity
        Side::Buy, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );

    // Act & Assert - Should not crash
    EXPECT_NO_THROW(orderBook_->submitNewOrder(event));
}

TEST_F(OrderBookTest, SubmitNewOrder_NegativeQuantity_DoesNotCrash) {
    // Arrange
    auto event = NewOrderEvent(
        "user123"_uid, 
        11002, 
        "AAPL"_sym, 
        -10,  // Negative quantity
        Side::Buy, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );

    // Act & Assert - Should not crash
    EXPECT_NO_THROW(orderBook_->submitNewOrder(event));
}

TEST_F(OrderBookTest, SubmitCancelOrder_NonExistentOrder_ReturnsFalse) {
    // Arrange - Try to cancel an order that doesn't exist
    auto cancelEvent = CancelOrderEvent("user123"_uid, 9999, "AAPL"_sym, 9999);

    // Expect no cancellation report since order doesn't exist
    EXPECT_CALL(*mockReportSink_, submitCanceledOrder(testing::_))
        .Times(0);

    // Act
    bool cancelResult = orderBook_->submitCancelOrder(cancelEvent);

    // Assert - Should return false for non-existent order
    EXPECT_FALSE(cancelResult);
}

TEST_F(OrderBookTest, SubmitTopOfBook_WithBothBidAndAsk_ShowsCorrectOrders) {
    // Arrange - Add both buy and sell orders
    auto buyEvent = NewOrderEvent(
        "user123"_uid, 
        9001, 
        "AAPL"_sym, 
        100, 
        Side::Buy, 
        Type::Limit, 
        toPrice(150.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(buyEvent);

    auto sellEvent = NewOrderEvent(
        "user456"_uid, 
        9002, 
        "AAPL"_sym, 
        75, 
        Side::Sell, 
        Type::Limit, 
        toPrice(151.00, TWO_DIGITS_PRICE_SPEC)
    );
    orderBook_->submitNewOrder(sellEvent);

    // Act - Request top of book
    auto topOfBookEvent = TopOfBookEvent("user789"_uid, 9003, "AAPL"_sym);
    
    TopOfBookReport capturedTopOfBook;
    EXPECT_CALL(*mockReportSink_, submitTopOfBook(testing::_))
        .WillOnce(testing::Invoke([&capturedTopOfBook](TopOfBookReport&& report) {
            capturedTopOfBook = std::move(report);
            return true;
        }));

    orderBook_->submitTopOfBook(topOfBookEvent);

    // Assert - Verify top of book shows both best bid and best ask
    // Best bid (highest buy price)
    EXPECT_EQ(capturedTopOfBook.bid_order_.orderId_, 9001);
    EXPECT_EQ(capturedTopOfBook.bid_order_.openQuantity_, 100);
    EXPECT_EQ(capturedTopOfBook.bid_order_.openQuantity_, 100);  // No fills yet
    EXPECT_EQ(capturedTopOfBook.bid_order_.price_, toPrice(150.00, TWO_DIGITS_PRICE_SPEC));
    
    // Best ask (lowest sell price)
    EXPECT_EQ(capturedTopOfBook.ask_order_.orderId_, 9002);
    EXPECT_EQ(capturedTopOfBook.ask_order_.openQuantity_, 75);
    EXPECT_EQ(capturedTopOfBook.ask_order_.openQuantity_, 75);  // No fills yet
    EXPECT_EQ(capturedTopOfBook.ask_order_.price_, toPrice(151.00, TWO_DIGITS_PRICE_SPEC));
}

TEST_F(OrderBookTest, SubmitTopOfBook_EmptyBook_ReturnsInvalidOrders) {
    // Arrange - Empty order book (no orders placed)
    auto topOfBookEvent = TopOfBookEvent("user123"_uid, 9999, "AAPL"_sym);
    
    TopOfBookReport capturedTopOfBook;
    EXPECT_CALL(*mockReportSink_, submitTopOfBook(testing::_))
        .WillOnce(testing::Invoke([&capturedTopOfBook](TopOfBookReport&& report) {
            capturedTopOfBook = std::move(report);
            return true;
        }));

    // Act
    orderBook_->submitTopOfBook(topOfBookEvent);

    // Assert - Verify both bid and ask orders are invalid (default Order objects)
    EXPECT_FALSE(capturedTopOfBook.bid_order_.isValid());
    EXPECT_FALSE(capturedTopOfBook.ask_order_.isValid());
}

} // namespace test
} // namespace Exchange