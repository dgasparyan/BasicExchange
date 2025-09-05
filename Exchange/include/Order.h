#ifndef ORDER_H
#define ORDER_H

#include "OrderUtils.h"


namespace Exchange {

class NewOrderEvent;

enum class OrderState {
  New = 0,
  PartiallyFilled,
  Filled,
  Cancelled
};

class Order {
  public:
    Order (const NewOrderEvent& event, SequenceNumber sequenceNumber) noexcept;

    // Order(UserId userId, OrderId clientOrderId, Symbol symbol, Quantity quantity, Side side, 
    //               /*Type type,*/ Price price) noexcept; 


  private:

    OrderState state_ {OrderState::New};


    UserId userId_ {INVALID_USER_ID};
    OrderId clientOrderId_ {};
    Symbol symbol_ {};


    Quantity openQuantity_ {INVALID_QUANTITY};
    Quantity filledQuantity_ {INVALID_QUANTITY};
    // Quantity cancelledQuantity_ {INVALID_QUANTITY};


    Side side_ {Side::Invalid};

    Type type_ {Type::Invalid};
    Price price_ {INVALID_PRICE};

    Timestamp timestamp_ {std::chrono::steady_clock::now()};

    // Used as a tie-breaker for orders with the same price/timestamp
    // we don't have persistance (yet) so this is good enough for now
    SequenceNumber sequenceNumber_ {0};
};

} // namespace Exchange
#endif