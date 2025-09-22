#ifndef ORDER_BOOK_MANAGER_H
#define ORDER_BOOK_MANAGER_H

#include <unordered_map>
#include <thread>
#include <semaphore>
#include <atomic>
#include <vector>
#include <latch>

#include <boost/lockfree/queue.hpp>

#include "OrderBook.h"
#include "Event.h"
#include "OrderUtils.h"
#include "ConfigManager.h"
#include "ReportSink.h"

namespace Exchange {

  enum class SubmitResult {
    Success,
    QueueFull,
    StopRequested,
  };

  class IOrderBookManager {
  public:
    virtual ~IOrderBookManager() = 0;

    virtual SubmitResult submit(const Event& event) = 0;

  };

class OrderBookManager : public IOrderBookManager {
public:
    using OrderBookMap = std::unordered_map<Symbol, std::unique_ptr<IOrderBook>>;

    OrderBookManager(const std::vector<Symbol>& symbols,
      IOrderBookFactory& factory, int numShards = std::thread::hardware_concurrency() / 2,
                                  int maxSubmitRetries = 64);

    ~OrderBookManager();

    SubmitResult submit(const Event& event) override;

    void stop();

private:

    struct Shard {
      static constexpr unsigned QUEUE_CAPACITY = 1024;
      explicit Shard(int maxSubmitRetries) : maxSubmitRetries_(maxSubmitRetries) {}

      void start(std::latch& started);
      void stop();
      void join();


      SubmitResult submit(const Event& event);
      void processEvents();
      void processEvent(Event&& event);



      static_assert(std::is_trivially_copyable_v<Event>, "Event must be trivially copyable for lock-free queue");
      boost::lockfree::queue<Event, boost::lockfree::capacity<QUEUE_CAPACITY>> eventQueue_ {};
      std::counting_semaphore<> semaphore_{ 0};
      std::atomic<bool> stopRequested_ {false};
      int maxSubmitRetries_ {0};

      OrderBookMap orderBooks_; 
      // kust be initialized fully before we access cuz 
      // going to do it concurrently
      // so we can't have any data races
      std::jthread thread_;
    };

    size_t shardIdx(Symbol symbol) const;

    std::atomic<bool> stopRequested_ {false};
    std::vector<std::unique_ptr<Shard>> shards_;

};

} // namespace Exchange

#endif // ORDER_BOOK_MANAGER_H 