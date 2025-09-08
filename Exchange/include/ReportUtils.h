#ifndef REPORT_UTILS_H
#define REPORT_UTILS_H

#include <vector>
#include "Order.h"

namespace Exchange {

enum class CancelReason {
  Fill_And_Kill,
  Other
};

struct CanceledOrderReport {
  OrderId orderId;
  Quantity remainingQuantity;
  CancelReason reason;
};


} // namespace Exchange

#endif // REPORT_UTILS_H