#ifndef ORDER_UTILS_H
#define ORDER_UTILS_H

#include <string>

namespace Exchange {
  // keep FIX api values

  enum class Type {
    Market = 1,
    Limit = 2,
    // Stop = 3,
    // StopLimit = 4,


    Invalid = 99,
  };
  enum class Side {
    Buy = 1,
    Sell,


    Invalid = 99,
  };

  Side toSide(std::string_view side);
  Type toType(std::string_view type);

  // TODO: change to a precise, comparatable type 
  using PriceType = double;
  constexpr PriceType INVALID_PRICE = -1;

  using OrderIdType = int;
  constexpr OrderIdType INVALID_ORDER_ID = -1;

  using QuantityType = int;
  constexpr QuantityType INVALID_QUANTITY = -1;


} // namespace Exchange
#endif