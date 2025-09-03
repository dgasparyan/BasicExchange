#ifndef ORDER_UTILS_H
#define ORDER_UTILS_H

#include <string>
#include "FixedString.h"

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


  #include <array>
  #include <string_view>
  #include <compare>
  #include <type_traits>
  #include <cstring>
  

  
  // TODO: change to a precise, comparatable type 
  using PriceType = double;
  constexpr PriceType INVALID_PRICE = -1;

  using OrderIdType = int;
  constexpr OrderIdType INVALID_ORDER_ID = -1;

  using QuantityType = int;
  constexpr QuantityType INVALID_QUANTITY = -1;

  using UserIdType = FixedString<32>;
  constexpr UserIdType INVALID_USER_ID {};
  constexpr UserIdType operator"" _uid(const char* s, std::size_t n) {
      return UserIdType(std::string_view{s, n}); // no unbounded scan, compile-time length
  }
  

  using SymbolType = FixedString<8>;
  constexpr SymbolType INVALID_SYMBOL {};
  constexpr SymbolType operator"" _sym(const char* s, std::size_t n) {
      return SymbolType(std::string_view{s, n}); // no unbounded scan, compile-time length
  }



} // namespace Exchange
#endif