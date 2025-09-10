#ifndef REPORT_SINK_H
#define REPORT_SINK_H

#include <vector>
#include "Order.h"
#include "ReportUtils.h"

namespace Exchange {

class ReportSink {
public:
    bool submitFills(std::vector<std::pair<OrderId, Quantity>>&& fills);

    bool submitCanceledOrder(CanceledOrderReport&& report);

    bool submitTopOfBook(TopOfBookReport&& report);

};

} // namespace Exchange

#endif