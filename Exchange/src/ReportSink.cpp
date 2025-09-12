#include "ReportSink.h"
#include <iostream>
#include <syncstream>

namespace Exchange {
namespace {
  constexpr int MAX_ITEMS_PER_BATCH = 64;
}

ReportSink::ReportSink() {
  thread = std::jthread([this] {
    run();
  });
}

ReportSink::~ReportSink() {
  stop();
}

void ReportSink::stop() {
  bool expected = false;
  if (stopRequested_.compare_exchange_strong(expected, true)) {
    semaphore_.release();
    if (thread.joinable()) {
      thread.join();
    }
  }
}

void ReportSink::run() {
  while (true) {
    semaphore_.acquire();
    if (stopRequested_.load()) {
      break;
    } 

    QueueItem item;
    if (!queue_.pop(item)) {
      semaphore_.release();
      continue;
    }
    report(std::move(item));

    int count = 0;
    while (count < MAX_ITEMS_PER_BATCH && semaphore_.try_acquire()) {
      if (queue_.pop(item)) {
        report(std::move(item));
        count++;
    } else {
        semaphore_.release();
        break;
      }
    }
  }

  // DO drain the reports at the end
  QueueItem item;
  while (queue_.pop(item)) {
    report(std::move(item));
  }
}

bool ReportSink::submitFills(ExecutionReportCollection&& fills) {
  size_t numPushed = 0;
  for (auto& report : fills) {
    if (queue_.push(QueueItem(std::in_place_type<ExecutionReport>, std::move(report)))) {
      numPushed++;
    } else {
      // TODO: handle this case
      // either count and report drops or briefly spin
    }
  }
  if (numPushed > 0) {
    semaphore_.release(numPushed);
  }
  return numPushed > 0;
}

bool ReportSink::submitCanceledOrder(OrderCanceledReport&& report) {
  if (queue_.push(QueueItem(std::in_place_type<OrderCanceledReport>, std::move(report)))) {
    semaphore_.release();
    return true;
  }
  return false;
}

bool ReportSink::submitTopOfBook(TopOfBookReport&& report) {
  if (queue_.push(QueueItem(std::in_place_type<TopOfBookReport>, std::move(report)))) {
    semaphore_.release();
    return true;
  }
  return false;
}

void ReportSink::report(QueueItem&& item) {
  std::visit([](auto&& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, ExecutionReport> 
               || std::is_same_v<T, OrderCanceledReport> 
               || std::is_same_v<T, TopOfBookReport>) {
      std::osyncstream(std::cout) << std::format("{}", arg) << '\n';
  } else {
      std::osyncstream(std::cout) << "Unknown report type\n";
    }
  }, std::move(item));
}

} // namespace Exchange