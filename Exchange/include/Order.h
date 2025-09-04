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
    explicit Order (const NewOrderEvent& event) noexcept;

    // Order(UserIdType userId, OrderIdType clientOrderId, SymbolType symbol, QuantityType quantity, Side side, 
    //               /*Type type,*/ PriceType price) noexcept; 


  private:

    OrderState state_ {OrderState::New};


    UserIdType userId_ {INVALID_USER_ID};
    OrderIdType clientOrderId_ {};
    SymbolType symbol_ {};


    QuantityType openQuantity_ {INVALID_QUANTITY};
    QuantityType filledQuantity_ {INVALID_QUANTITY};
    // QuantityType cancelledQuantity_ {INVALID_QUANTITY};


    Side side_ {Side::Invalid};

    Type type_ {Type::Invalid};
    PriceType price_ {INVALID_PRICE};
};

} // namespace Exchange
#endif