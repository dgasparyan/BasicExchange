#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <string>
#include <thread>
#include <atomic>

#include "EventQueue.h"
#include "EventParser.h"
#include "OrderBookManager.h"


namespace Exchange {

class Exchange {
public:
    Exchange(EventQueue& eventQueue, EventParser& eventParser, IOrderBookManager& orderBookManager);
    ~Exchange();

    void start();


private:
    void processEvent(const std::string& event) noexcept;

    void requestStop();
    void handleStop();

  private:

    IOrderBookManager& orderBookManager_;


    EventParser& eventParser_;
    EventQueue& eventQueue_;
    std::unique_ptr<SubscriptionHandle> eventQueueSubscription_;


    std::atomic<bool> stopRequested_ {false};
};

} // namespace Exchange

#endif // EXCHANGE_H