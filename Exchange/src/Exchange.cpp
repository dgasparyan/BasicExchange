#include "Exchange.h"

#include <iostream>

#include "Event.h"
#include "EventParser.h"

namespace Exchange {


Exchange::Exchange(EventQueue& eventQueue, EventParser& eventParser, OrderBookManager& orderBookManager) : orderBookManager_(orderBookManager), eventParser_(eventParser), eventQueue_(eventQueue){
}

Exchange::~Exchange() {
  requestStop();
  handleStop();
}

void Exchange::start() {
  eventQueueSubscription_ = eventQueue_.subscribeWith(&Exchange::processEvent, this);

  std::unique_lock<std::mutex> lock(stopMutex_);
  
  stopCondition_.wait(lock, [this] { return stopRequested_; });
  handleStop();
}

void Exchange::requestStop() {
  {
    std::lock_guard<std::mutex> lock(stopMutex_);
    stopRequested_ = true;
  }
  stopCondition_.notify_all();
}

void Exchange::handleStop() {
  eventQueueSubscription_.reset();
  orderBookManager_.stop();
}

void Exchange::processEvent(const std::string& eventStr) {
    EventType eventType = eventParser_.getEventType(eventStr);
    
    switch (eventType) {
      case EventType::Quit:
        requestStop();
        break;
      case EventType::NewOrder:
      case EventType::CancelOrder:
      case EventType::TopOfBook:
        try {
          auto event = eventParser_.parse(eventStr);
          orderBookManager_.submit(std::move(event));
          // std::cout << "Processed event: " << toString(eventPtr->type()) << std::endl;
        } catch (const std::exception& e) {
          std::cerr << "Error processing event: " << e.what() << std::endl;
        }
        break;
      default:
        std::cerr << "Unknown event type: " << eventStr << std::endl;
        break;
    }
}

} // namespace Exchange