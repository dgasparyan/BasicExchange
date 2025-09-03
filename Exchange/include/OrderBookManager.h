#ifndef ORDER_BOOK_MANAGER_H
#define ORDER_BOOK_MANAGER_H

#include <unordered_map>
#include <queue>

#include <semaphore>

#include <mutex>
#include <thread>
#include <condition_variable>
#include <stop_token>

#include <boost/lockfree/queue.hpp>

#include "OrderBook.h"
#include "Event.h"

namespace Exchange {

  class IOrderBookManager {
  public:
    virtual ~IOrderBookManager() = 0;

    virtual bool submit(Event event) = 0;

  };

class OrderBookManager : public IOrderBookManager {
public:
    using OrderBookMap = std::unordered_map<std::string, std::unique_ptr<IOrderBook>>;

    OrderBookManager(OrderBookMap && map, int numThreads = std::thread::hardware_concurrency() / 2);

    ~OrderBookManager();

    bool submit(Event event) override;

    void stop();

  private:
    void processEvents();

    void processEvent(Event event);

  private:
    OrderBookMap orderBooks_;

    std::queue<Event> eventQueue_;

    // boost::lockfree::queue<Event> eventQueue_;

    std::mutex mutex_;
    std::condition_variable_any cv_;

    // std::counting_semaphore<> semaphore_{0};

    std::stop_source stopSource_;
    std::vector<std::jthread> threads_;
};

} // namespace Exchange

#endif // ORDER_BOOK_MANAGER_H 