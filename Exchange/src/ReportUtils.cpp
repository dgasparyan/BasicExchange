#include "ReportUtils.h"

namespace Exchange {

  
  TopOfBookReport::TopOfBookReport(Symbol symbol, OrderId requestId, const Order& bid, const Order& ask) 
  : symbol_(symbol), requestId_(requestId),
    bid_order_(bid.clientOrderId(), bid.price(), bid.openQuantity()),
    ask_order_(ask.clientOrderId(), ask.price(), ask.openQuantity()) {}



} // namespace Exchange