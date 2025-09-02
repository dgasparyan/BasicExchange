#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "EventQueue.h"
#include "EventParser.h"
#include "OrderBookManager.h"


namespace Exchange {

class Exchange {
public:
    Exchange(EventQueue& eventQueue, EventParser& eventParser, OrderBookManager& orderBookManager);
    ~Exchange();

    void start();


private:
    void processEvent(const std::string& event);

    void requestStop();
    void handleStop();

  private:

    OrderBookManager& orderBookManager_;


    EventParser& eventParser_;
    EventQueue& eventQueue_;
    std::unique_ptr<SubscriptionHandle> eventQueueSubscription_;


    // std::thread workerThread_;
    std::mutex stopMutex_;
    std::condition_variable stopCondition_;
    bool stopRequested_ = false;
};

} // namespace Exchange

#endif // EXCHANGE_H