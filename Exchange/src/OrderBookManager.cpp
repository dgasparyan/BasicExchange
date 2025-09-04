#include "OrderBookManager.h"
#include <iostream>
#include <thread>


namespace Exchange {

  namespace {

    constexpr unsigned MAX_BATCH_SIZE = 32;
    constexpr unsigned QUEUE_CAPACITY = 1024;


    void backoff(unsigned n) {
      if (n < 32) std::this_thread::yield();
      else std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
  
  }

IOrderBookManager::~IOrderBookManager() = default;

OrderBookManager::OrderBookManager(OrderBookManager::OrderBookMap&& map, int numThreads) 
: orderBooks_(std::move(map)), eventQueue_(QUEUE_CAPACITY) {
  
  // at least 2 threads otherwise what's even the point amirite
  numThreads = std::max(2, numThreads);
  std::cout << "OrderBookManager::OrderBookManager: numThreads: " << numThreads << std::endl;
  for (int i = 0; i < numThreads; ++i) {
    threads_.emplace_back([this]() { processEvents(); });
  }
}

OrderBookManager::~OrderBookManager() {
  stop();
}

void OrderBookManager::stop() {
  if (!stopRequested_.exchange(true)) {
    semaphore_.release(threads_.size());

    // a.k.a. join
    threads_.clear();
  }
}

void OrderBookManager::processEvents() {
  while (true) {
    semaphore_.acquire();
      if (stopRequested_.load()) {
        break;
      }
      Event event;

      unsigned int spinCount {0};
      while (!eventQueue_.pop(event)) {
        backoff(spinCount++);
      }
      processEvent(std::move(event));

      // opportunistic batching
      unsigned int grabbed {0};
      while (!stopRequested_.load() && grabbed < MAX_BATCH_SIZE && semaphore_.try_acquire()) {
          if (stopRequested_.load(std::memory_order_relaxed)) {
            semaphore_.release(); // return the token so a blocked worker can wake
            break;
          }

          if (eventQueue_.pop(event)) { processEvent(std::move(event)); ++grabbed; }
          else { semaphore_.release(1); break; } // return token if we lost the race
      }
      
      
    }
  }

void OrderBookManager::processEvent(Event event) {
  // orderBooks_[event->symbol()]->submit(std::move(event));
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

bool OrderBookManager::submit(Event event) {
  // return orderBooks_[event->symbol()]->submit(std::move(event));
  // std::cout << "OrderBookManager::submit" << std::endl;
  if (stopRequested_.load()) {
    return false;
  }

  if (!eventQueue_.push(std::move(event))) {
    // std::cout << "OrderBookManager::submit: Queue is full" << std::endl;
    return false;
  }
  semaphore_.release(1);

  return true;
}


} // namespace Exchange 