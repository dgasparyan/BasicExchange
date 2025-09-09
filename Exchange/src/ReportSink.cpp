#include "ReportSink.h"
#include <iostream>

namespace Exchange {

bool ReportSink::submitFills(std::vector<std::pair<OrderId, Quantity>>&& fills) {
  std::cout << "ReportSink::submit: " << fills.size() << std::endl;
  return true;  
}

bool ReportSink::submitCanceledOrder(CanceledOrderReport&& report) {
  std::cout << "ReportSink::submitCanceledOrder: " << report.orderId << " " << report.remainingQuantity << " " << static_cast<int>(report.reason) << std::endl;
  return true;
}

} // namespace Exchange