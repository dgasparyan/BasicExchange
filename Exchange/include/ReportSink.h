#ifndef REPORT_SINK_H
#define REPORT_SINK_H

#include <atomic>
#include <semaphore>
#include <thread>
#include <variant>

#include <boost/lockfree/spsc_queue.hpp>

#include "Order.h"
#include "ReportUtils.h"


namespace Exchange {

class ReportSink {
public:
    ReportSink();
    ~ReportSink();

    bool submitFills(ExecutionReportCollection&& fills);

    bool submitCanceledOrder(OrderCanceledReport&& report);

    bool submitTopOfBook(TopOfBookReport&& report);
private:

  using QueueItem = std::variant<std::monostate, ExecutionReport, OrderCanceledReport, TopOfBookReport>;

  void stop();
  void run();
  void report(QueueItem&& item);


  boost::lockfree::spsc_queue<QueueItem> queue_{1024};
  std::atomic<bool> stopRequested_ {false};
  std::counting_semaphore<> semaphore_ {0};

  std::jthread thread;
};

} // namespace Exchange

#endif