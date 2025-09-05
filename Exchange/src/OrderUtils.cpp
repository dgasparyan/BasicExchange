#include "OrderUtils.h"
#include "CommonUtils.h"

namespace Exchange {

namespace {
  static const char* BUY = "BUY";
  static const char* SELL = "SELL";

  static const char* MARKET = "MARKET";
  static const char* LIMIT = "LIMIT";
}

Side toSide(std::string_view side) {
    auto upperSide = trimAndUpperCopy(side);
    
    if (upperSide == BUY || upperSide == "1") {
        return Side::Buy;
    } else if (upperSide == SELL || upperSide == "2") {
        return Side::Sell;
    }
    
    return Side::Invalid;
}

Type toType(std::string_view type) {
    auto upperType = trimAndUpperCopy(type);
    
    if (upperType == MARKET || upperType == "1") {
        return Type::Market;
    } else if (upperType == LIMIT || upperType == "2") {
        return Type::Limit;
    }
    
    return Type::Invalid;
}

PriceType toPrice(double price, const PriceSpec& spec) {
  // use long double for fewer rounding surprises in the multiply
  long double scaled = static_cast<long double>(price) * spec.scale;
  // Round to nearest scale unit
  auto scaled_i = static_cast<int64_t>(llround(scaled));
  // Must be on a tick grid
  if (scaled_i % spec.tick_scaled != 0) throw std::invalid_argument("Price is not on a tick grid");
  // store in *ticks*
  return PriceType{ scaled_i / spec.tick_scaled };
}

} // namespace Exchange 