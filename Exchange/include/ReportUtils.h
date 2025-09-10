#ifndef REPORT_UTILS_H
#define REPORT_UTILS_H

#include <vector>
#include "Order.h"

namespace Exchange {


  // TODO: report properly
  using PFillData = std::pair<OrderId, Quantity>;
  using PFillVec = std::vector<PFillData>;

enum class CancelReason {
  Fill_And_Kill,
  User_Canceled,
  Other
};

struct CanceledOrderReport {
  OrderId orderId;
  Quantity remainingQuantity;
  CancelReason reason;
};

struct TopOfBookReport {
  Order bid_order;
  Order ask_order;
};


} // namespace Exchange

#endif // REPORT_UTILS_H