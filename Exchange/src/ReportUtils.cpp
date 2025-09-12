#include "ReportUtils.h"

namespace Exchange {

  
  TopOfBookReport::TopOfBookReport(Symbol symbol, const Order& bid, const Order& ask) 
  : symbol_(symbol),
    bid_order_(bid.clientOrderId(), bid.price(), bid.openQuantity()),
    ask_order_(ask.clientOrderId(), ask.price(), ask.openQuantity()) {}



} // namespace Exchange