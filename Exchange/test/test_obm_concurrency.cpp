#include <gtest/gtest.h>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <optional>
#include <memory>
#include <thread>
#include <chrono>
#include <latch>

#include "OrderBook.h"
#include "Event.h"
#include "OrderBookManager.h"
#include "OrderUtils.h"

namespace Exchange {
namespace test {

// // hashing values are (hash(symbol) % 5 that is)
//   AAPL 4
//   GOOGL 2
//   MSFT 4
//   AMZN 1
//   META 0
//   NVDA 3
//   TSLA 2
//   NFLX 4
//   JPM 1
//   V 1
static const std::vector<Symbol> kTestSymbols = {
    "AAPL"_sym,
    "GOOGL"_sym,
    "MSFT"_sym,
    "AMZN"_sym,
    "META"_sym,
    "NVDA"_sym,
    "TSLA"_sym,
    "NFLX"_sym,
    "JPM"_sym,
    "V"_sym
};

  

class MockOrderBook : public IOrderBook {
public:
    std::atomic<uint64_t> newOrderCalls_{0};
    std::atomic<uint64_t> cancelOrderCalls_{0};
    std::atomic<uint64_t> topOfBookCalls_{0};

    bool defaultNewOrderReturn_{true};
    bool defaultCancelOrderReturn_{true};

    bool submitNewOrder(const NewOrderEvent& /*event*/) override {
        newOrderCalls_.fetch_add(1, std::memory_order_relaxed);
        return defaultNewOrderReturn_;
    }

    bool submitCancelOrder(const CancelOrderEvent& /*event*/) override {
        cancelOrderCalls_.fetch_add(1, std::memory_order_relaxed);
        return defaultCancelOrderReturn_;
    }

    void submitTopOfBook(const TopOfBookEvent& /*event*/) override {
        topOfBookCalls_.fetch_add(1, std::memory_order_relaxed);
    }

    void reset() {
        newOrderCalls_ = 0;
        cancelOrderCalls_ = 0;
        topOfBookCalls_ = 0;
    }
};

class TestOrderBookFactory : public IOrderBookFactory {
public:
    std::unique_ptr<IOrderBook> create(Symbol symbol) override {
      auto mb = std::make_unique<MockOrderBook>();
      mbMap_[symbol] = mb.get();
      return mb;
    }
    std::unordered_map<Symbol, MockOrderBook*> mbMap_;
};


inline OrderId nextClientOrderId() {
  static std::atomic<OrderId> g_nextClientOrderId{1000};
  return ++g_nextClientOrderId;
}

// Helpers to create NewOrderEvent with optional args
inline Event makeNewOrder(
    Symbol symbol = "AAPL"_sym,
    Quantity quantity = 100,
    Side side = Side::Buy,
    Type type = Type::Limit,
    std::optional<Price> price = std::nullopt,
    UserId userId = "user"_uid,
    OrderId clientOrderId = 0)
{
    if (clientOrderId == 0) clientOrderId = nextClientOrderId();
    if (price.has_value()) {
        return Event(std::in_place_type<NewOrderEvent>, userId, clientOrderId, symbol, quantity, side, type, *price);
    }
    return Event(std::in_place_type<NewOrderEvent>, userId, clientOrderId, symbol, quantity, side, type);
}

inline Event makeLimit(
    Symbol symbol,
    Quantity quantity,
    Side side,
    Price price,
    UserId userId = "user"_uid,
    OrderId clientOrderId = 0)
{
    return makeNewOrder(symbol, quantity, side, Type::Limit, price, userId, clientOrderId);
}

inline Event makeMarket(
    Symbol symbol,
    Quantity quantity,
    Side side,
    UserId userId = "user"_uid,
    OrderId clientOrderId = 0)
{
    return makeNewOrder(symbol, quantity, side, Type::Market, std::nullopt, userId, clientOrderId);
}

TEST(OBMConcurrencyTest, Ctor_race) {

  std::atomic<OrderBookManager*> aObm {nullptr};
  std::vector<std::jthread> threads;
  std::atomic<int> numFailed {0};
  for (int i = 0; i < 5; ++i) {
    threads.emplace_back([&]() {
      for (int i = 0; i < 100; ++i) {
        aObm.wait(nullptr);
        if (aObm.load()->submit(makeNewOrder(Symbol{"V"})) != SubmitResult::Success) {
          numFailed.fetch_add(1);
        }
      }
    });
  }
  

  TestOrderBookFactory factory;
  constexpr int numThreads = 5;
  OrderBookManager obm(kTestSymbols, factory, numThreads);
  aObm.store(&obm);
  aObm.notify_all();

  for (auto& thread : threads) {
    thread.join();
  }

  obm.stop();

  EXPECT_EQ(factory.mbMap_["V"_sym]->newOrderCalls_.load(std::memory_order_relaxed), 500);
  EXPECT_EQ(factory.mbMap_["V"_sym]->cancelOrderCalls_.load(std::memory_order_relaxed), 0);
  EXPECT_EQ(factory.mbMap_["V"_sym]->topOfBookCalls_.load(std::memory_order_relaxed), 0);
}

TEST(OBMConcurrencyTest, Stop_race) {

  TestOrderBookFactory factory;
  constexpr int numThreads = 5;
  OrderBookManager obm(kTestSymbols, factory, numThreads, 10);

  constexpr int kNumEvents = 250;
  constexpr int kNumThreads = 5;

  std::vector<std::jthread> threads;
  std::atomic<int> numStopRequested {0};
  std::atomic<int> numSent {0};
  std::latch started(kNumThreads);
  std::atomic<bool> stopRequested {false};
  for (int i = 0; i < kNumThreads; ++i) {
    threads.emplace_back([&]() {
      started.arrive_and_wait();
      int numSent_ = 0;
      int numStopRequested_ = 0;
      for (int i = 0; i < kNumEvents; ++i) {
        auto symbol = i % 2 == 0 ? Symbol{"AAPL"} : Symbol{"MSFT"};
        auto result = obm.submit(makeNewOrder(symbol));
        if (result == SubmitResult::Success) {
          numSent_++;
        } else if (result == SubmitResult::StopRequested) {
          numStopRequested_++;
        }

        if (numSent_ % 50 == 0) {
          stopRequested.store(true, std::memory_order_release);
          stopRequested.notify_one();
        }
      }
      numSent.fetch_add(numSent_);
      numStopRequested.fetch_add(numStopRequested_);
    });
  }

  while (!stopRequested.load(std::memory_order_acquire)) {
    stopRequested.wait(false, std::memory_order_acquire);
  }
  obm.stop();

  for (auto& thread : threads) {
    thread.join();
  }

  if (numStopRequested.load() == 0) {
    GTEST_SKIP() << "All events were submitted before stop; test inconclusive";
  }

  std::cout << "numStopRequested: " << numStopRequested.load() << std::endl;
  std::cout << "numSent: " << numSent.load() << std::endl;
  auto aaplEvents = factory.mbMap_["AAPL"_sym]->newOrderCalls_.load(std::memory_order_relaxed);
  auto msftEvents = factory.mbMap_["MSFT"_sym]->newOrderCalls_.load(std::memory_order_relaxed);
  EXPECT_EQ(aaplEvents + msftEvents + numStopRequested.load(), kNumEvents * kNumThreads);

}


TEST(OBMConcurrencyTest, Heavy_submit_load_no_drops_with_high_retries) {

  constexpr int kNumEvents = 2500;
  constexpr int kNumThreads = 5;

  TestOrderBookFactory factory;
  constexpr int numThreads = 5;
  OrderBookManager obm(kTestSymbols, factory, numThreads, kNumEvents);

  std::vector<std::jthread> threads;
  std::latch started(kNumThreads);
  std::atomic<int> numFailed {0};
  std::atomic<int> numQueueFull {0};
  for (int i = 0; i < kNumThreads; ++i) {
    threads.emplace_back([&]() {
      started.arrive_and_wait();
      int queueFull = 0;
      for (int i = 0; i < kNumEvents; ++i) {
        auto symbol = i % 2 == 0 ? Symbol{"AAPL"} : Symbol{"MSFT"};
        auto result = obm.submit(makeNewOrder(symbol));
        if (result == SubmitResult::QueueFull) {
          queueFull++;
        }
      }
      numQueueFull.fetch_add(queueFull);
    });
  };


  for (auto& thread : threads) {
    thread.join();
  }

  obm.stop();


  auto aaplEvents = factory.mbMap_["AAPL"_sym]->newOrderCalls_.load(std::memory_order_relaxed);
  auto msftEvents = factory.mbMap_["MSFT"_sym]->newOrderCalls_.load(std::memory_order_relaxed);
  EXPECT_EQ(aaplEvents + msftEvents, kNumEvents * kNumThreads);
  EXPECT_EQ(numQueueFull.load(), 0);

}

TEST(OBMConcurrencyTest, Heavy_submit_load_might_drop_with_low_retries) {

  constexpr int kNumEvents = 2500;
  constexpr int kNumThreads = 5;
  constexpr int kMaxSubmitRetries = 5;

  TestOrderBookFactory factory;
  constexpr int numThreads = 5;
  OrderBookManager obm(kTestSymbols, factory, numThreads, kMaxSubmitRetries);

  std::vector<std::jthread> threads;
  std::latch started(kNumThreads);
  std::atomic<int> numFailed {0};
  std::atomic<int> numQueueFull {0};
  for (int i = 0; i < kNumThreads; ++i) {
    threads.emplace_back([&]() {
      started.arrive_and_wait();
      int queueFull = 0;
      for (int i = 0; i < kNumEvents; ++i) {
        auto symbol = i % 2 == 0 ? Symbol{"AAPL"} : Symbol{"MSFT"};
        auto result = obm.submit(makeNewOrder(symbol));
        if (result == SubmitResult::QueueFull) {
          queueFull++;
        }
      }
      numQueueFull.fetch_add(queueFull);
    });
  };


  for (auto& thread : threads) {
    thread.join();
  }

  obm.stop();


  auto aaplEvents = factory.mbMap_["AAPL"_sym]->newOrderCalls_.load(std::memory_order_relaxed);
  auto msftEvents = factory.mbMap_["MSFT"_sym]->newOrderCalls_.load(std::memory_order_relaxed);
  EXPECT_EQ(aaplEvents + msftEvents + numQueueFull.load(), kNumEvents * kNumThreads);
  EXPECT_NE(numQueueFull.load(), 0);

}



} // namespace test
} // namespace Exchange

