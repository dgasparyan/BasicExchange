#include "OrderBookManager.h"
#include <iostream>
#include <thread>


namespace Exchange {

  namespace {

    constexpr unsigned MAX_BATCH_SIZE = 128;


    
    void backoff(unsigned n) {
      if (n < 32) std::this_thread::yield();
      else std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

  }

IOrderBookManager::~IOrderBookManager() = default;

OrderBookManager::OrderBookManager(const std::vector<Symbol>& symbols, IOrderBookFactory& factory, int numShards, int maxSubmitRetries) 
{  
  numShards = std::max(2, numShards);
  shards_.reserve(numShards);  
  for (int i = 0; i < numShards; ++i) {
    shards_.emplace_back(std::make_unique<Shard>(maxSubmitRetries));
  }

  for (const auto& symbol : symbols) {
    auto& shard = shards_[shardIdx(symbol)];
    shard->orderBooks_[symbol] = factory.create(symbol);
  }

  std::latch started(numShards);
  std::ranges::for_each(shards_, [&started](auto& shard) { shard->start(started); });
  started.wait();
}

OrderBookManager::~OrderBookManager() noexcept {
  stop();
}

void OrderBookManager::stop() noexcept {
  if (!stopRequested_.exchange(true)) {
    for (auto& shard : shards_) {
      shard->stop();
    }
    for (auto& shard : shards_) {
      shard->join();
    }
  }
}


SubmitResult OrderBookManager::submit(const Event& event) {
  if (stopRequested_.load()) {
    return SubmitResult::StopRequested;
  }

  auto idx  {shardIdx(event.symbol())};
  return shards_[idx]->submit(event);
}

size_t OrderBookManager::shardIdx(Symbol symbol) const {
  return std::hash<Symbol>()(symbol) % shards_.size();
}

void OrderBookManager::Shard::start(std::latch& started) {
  thread_ = std::jthread([this, &started]() { started.count_down(); processEvents();  });
}

void OrderBookManager::Shard::stop() noexcept {
  if (!stopRequested_.exchange(true)) {
    semaphore_.release(1);
  }
}

void OrderBookManager::Shard::join() noexcept {
  if (thread_.joinable()) {
    thread_.join();
  }
}

SubmitResult OrderBookManager::Shard::submit(const Event& event) {
  if (stopRequested_.load()) {
    return SubmitResult::StopRequested;
  }

  if (eventQueue_.push(event)) {
    semaphore_.release(1);
    return SubmitResult::Success;
  }

  int numTries = 0;
  while (true) {
    if (stopRequested_.load()) {
      return SubmitResult::StopRequested;
    }
    if (eventQueue_.push(event)) {
      semaphore_.release(1);
      return SubmitResult::Success;
    }


    backoff(numTries++);
    if (numTries > maxSubmitRetries_) {
      return SubmitResult::QueueFull;
    }
  }

}

void OrderBookManager::Shard::processEvents() noexcept {
  try {
    auto handleSafely = [this](Event&& ev) {
      try {
        processEvent(std::move(ev));
      } catch (const std::exception& e) {
        std::cerr << "OrderBookManager::Shard::processEvents: error processing event: "
                  << e.what() << std::endl;
      } catch (...) {
        std::cerr << "OrderBookManager::Shard::processEvents: unknown error processing event"
                  << std::endl;
      }
    };
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
        handleSafely(std::move(event));

        // opportunistic batching
        unsigned int grabbed {0};
        while (!stopRequested_.load() && grabbed < MAX_BATCH_SIZE && semaphore_.try_acquire()) {
            if (stopRequested_.load()) {
              semaphore_.release(); // return the token so a blocked worker can wake
              break;
            }

            if (eventQueue_.pop(event)) { handleSafely(std::move(event)); ++grabbed; }
            else { semaphore_.release(1); break; } // return token if we lost the race
        }
      }

      Event event;
      while (eventQueue_.pop(event)) {
        handleSafely(std::move(event));
      }
  } catch (const std::exception& e) {
    std::cerr << "OrderBookManager::Shard::processEvents: " << e.what() << std::endl;
  }
}

void OrderBookManager::Shard::processEvent(Event&& arg) {
  auto event = std::move(arg.data_);

  auto findAndInvoke = [this](auto&& event, auto&& memFunc) {
    auto it = orderBooks_.find(event.symbol());
    if (it != orderBooks_.end()) {
      std::invoke(memFunc, (*it->second), std::forward<decltype(event)>(event));
    }
    else {
      std::cout << "OrderBookManager::processEvent: Symbol not found: " << toString(event.eventType()) << " " << event.symbol() << std::endl;
    }
  };

  std::visit([findAndInvoke](auto&& event) {
    using T = std::decay_t<decltype(event)>;
    if constexpr (std::is_same_v<T, NewOrderEvent>) {
      findAndInvoke(std::forward<decltype(event)>(event), &IOrderBook::submitNewOrder);
    }
    else if constexpr (std::is_same_v<T, CancelOrderEvent>) {
      findAndInvoke(std::forward<decltype(event)>(event), &IOrderBook::submitCancelOrder);
    }
    else if constexpr (std::is_same_v<T, TopOfBookEvent>) {
      findAndInvoke(std::forward<decltype(event)>(event), &IOrderBook::submitTopOfBook);
    } 
    else
        std::cout << "Unknown Event "  << '\n';
  }, std::move(event));

}

} // namespace Exchange 