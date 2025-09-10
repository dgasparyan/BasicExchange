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
    Order() = default;
    Order (const NewOrderEvent& event, SequenceNumber sequenceNumber) noexcept;
    Order (const NewOrderEvent& event, SequenceNumber sequenceNumber, Quantity filledQuantity) noexcept;

    bool isValid() const;
    bool isActive() const;

    OrderState state() const { return state_; }


    UserId userId() const { return userId_; }
    OrderId clientOrderId() const { return clientOrderId_; }
    Symbol symbol() const { return symbol_; }
    Side side() const { return side_; }
    Type type() const { return type_; }
    Price price() const { return price_; }
    Timestamp timestamp() const { return timestamp_; }
    SequenceNumber sequenceNumber() const { return sequenceNumber_; }

    Quantity quantity() const { return quantity_; }
    Quantity openQuantity() const { return openQuantity_; }
    Quantity filledQuantity() const { return quantity() - openQuantity(); }

    Quantity fill(Quantity quantity);

    void cancel();


  private:

    OrderState state_ {OrderState::New};


    UserId userId_ {INVALID_USER_ID};
    OrderId clientOrderId_ {INVALID_ORDER_ID};
    Symbol symbol_ {};


    Quantity quantity_ {INVALID_QUANTITY};
    Quantity openQuantity_ {INVALID_QUANTITY};
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