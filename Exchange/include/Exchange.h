#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "EventQueue.h"
#include "EventParser.h"

namespace Exchange {

class Exchange {
public:
    Exchange(EventQueue& eventQueue, EventParser& eventParser);
    ~Exchange();

    void start();

    void stop();

private:
    void processEvent(const std::string& event);

private:
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