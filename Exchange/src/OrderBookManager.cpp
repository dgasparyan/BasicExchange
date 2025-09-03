#include "OrderBookManager.h"
#include <iostream>
#include <thread>


namespace Exchange {


IOrderBookManager::~IOrderBookManager() = default;

OrderBookManager::OrderBookManager(OrderBookManager::OrderBookMap&& map, int numThreads) : orderBooks_(std::move(map)) {
  std::cout << "OrderBookManager::OrderBookManager: numThreads: " << numThreads << std::endl;
  for (int i = 0; i < numThreads; ++i) {
    threads_.emplace_back([this]() { processEvents(); });
  }
}

OrderBookManager::~OrderBookManager() {
  stop();
}


void OrderBookManager::stop() {
  stopSource_.request_stop();
  cv_.notify_all();

  // manually join here, we don't want it processing stuff after it's stoppped
}

void OrderBookManager::processEvents() {
  while (true) {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, stopSource_.get_token(), [this] { return !eventQueue_.empty(); });
      if (stopSource_.stop_requested()) {
        break;
      }

      // process all of the events
      while (true) {
        // std::unique_lock<std::mutx> lock(mutex_);
        std::cout << std::this_thread::get_id() << " OrderBookManager::processEvents: Queue size: " << eventQueue_.size() << std::endl;
        if (eventQueue_.empty()) {
          break;
        }
        auto event = std::move(eventQueue_.front());
        eventQueue_.pop();
        lock.unlock();
        // TODO: try/catch here
        processEvent(std::move(event));
        lock.lock();
    }
  }
}

void OrderBookManager::processEvent(Event event) {
  // orderBooks_[event->symbol()]->submit(std::move(event));
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

bool OrderBookManager::submit(Event event) {
  // return orderBooks_[event->symbol()]->submit(std::move(event));
  std::cout << "OrderBookManager::submit" << std::endl;
  if (stopSource_.stop_requested()) {
    return false;
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);
    eventQueue_.push(std::move(event));
    cv_.notify_all();
  }

  return true;
}


} // namespace Exchange 