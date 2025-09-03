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

} // namespace Exchange 