#ifndef ORDER_BOOK_MANAGER_H
#define ORDER_BOOK_MANAGER_H

#include <unordered_map>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <stop_token>

#include "OrderBook.h"
#include "Event.h"

namespace Exchange {

  class IOrderBookManager {
  public:
    virtual ~IOrderBookManager() = 0;

    virtual bool submit(std::unique_ptr<OrderEvent> event) = 0;

  };

class OrderBookManager : public IOrderBookManager {
public:
    using OrderBookMap = std::unordered_map<std::string, std::unique_ptr<IOrderBook>>;

    OrderBookManager(OrderBookMap && map, int numThreads = std::thread::hardware_concurrency() / 2);

    ~OrderBookManager();

    bool submit(std::unique_ptr<OrderEvent> event) override;

    void stop();

  private:
    void processEvents();

    void processEvent(std::unique_ptr<OrderEvent> event);

  private:
    OrderBookMap orderBooks_;

    std::queue<std::unique_ptr<OrderEvent>> eventQueue_;

    std::mutex mutex_;
    std::condition_variable_any cv_;

    std::stop_source stopSource_;
    std::vector<std::jthread> threads_;
};

} // namespace Exchange

#endif // ORDER_BOOK_MANAGER_H 