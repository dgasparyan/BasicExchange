#ifndef ORDER_UTILS_H
#define ORDER_UTILS_H

#include <string>
#include <chrono>

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
  

  // Price is an integer number of *ticks*
struct Price {
  int64_t ticks{};                         // e.g., $10.53 with tick=$0.01 → 1053 ticks
  friend constexpr auto operator<=>(const Price&, const Price&) = default;
  // implicit conversions are risky; keep this a tiny strong type
};

// Per-instrument price spec: scale and tick (both in integer "scale units")
struct PriceSpec {
  // scale = how many "scale units" per 1.00 of currency (choose so tick_size becomes integer)
  // examples:
  //   equities: scale=100  (1 = $0.01)
  //   fx (pips): scale=10000 (1 = 0.0001)
  //   crypto: scale=100000000 (1 = 1e-8)
  // tick_scaled = tick size expressed in scale units
  int32_t scale;          // >0
  int32_t tick_scaled;    // >0, divides any valid scaled price exactly
};

inline constexpr PriceSpec TWO_DIGITS_PRICE_SPEC = PriceSpec{100, 1};
  
constexpr Price INVALID_PRICE {Price{-1}};
constexpr Price MARKET_PRICE {Price{std::numeric_limits<int64_t>::max()}};

Price toPrice(double price, const PriceSpec& spec);


  using OrderId = int;
  constexpr OrderId INVALID_ORDER_ID = -1;

  using Quantity = int;
  constexpr Quantity INVALID_QUANTITY = -1;

  using UserId = FixedString<32>;
  constexpr UserId INVALID_USER_ID {};
  constexpr UserId operator"" _uid(const char* s, std::size_t n) {
      return UserId(std::string_view{s, n}); // no unbounded scan, compile-time length
  }
  

  using Symbol = FixedString<8>;
  constexpr Symbol INVALID_SYMBOL {};
  constexpr Symbol operator"" _sym(const char* s, std::size_t n) {
      return Symbol(std::string_view{s, n}); // no unbounded scan, compile-time length
  }

  using Timestamp = std::chrono::steady_clock::time_point;

  using SequenceNumber = uint64_t;

} // namespace Exchange
#endif