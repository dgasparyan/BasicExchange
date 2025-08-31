#include "Exchange.h"

#include <iostream>

#include "Event.h"
#include "EventParser.h"

namespace Exchange {


Exchange::Exchange(EventQueue& eventQueue, EventParser& eventParser) : eventParser_(eventParser), eventQueue_(eventQueue){
}

Exchange::~Exchange() {
  stop();
}

void Exchange::start() {
  eventQueueSubscription_ = eventQueue_.subscribeWith(&Exchange::processEvent, this);

  std::unique_lock<std::mutex> lock(stopMutex_);
  stopCondition_.wait(lock, [this] { return stopRequested_; });
  eventQueueSubscription_.reset();
}

void Exchange::stop() {
  {
    std::lock_guard<std::mutex> lock(stopMutex_);
    stopRequested_ = true;
  }
  stopCondition_.notify_all();
}

void Exchange::processEvent(const std::string& event) {
    EventType eventType = eventParser_.getEventType(event);
    
    switch (eventType) {
      case EventType::Quit:
        stop();
        break;
      case EventType::NewOrder:
      case EventType::CancelOrder:
      case EventType::TopOfBook:
        try {
          auto eventPtr = eventParser_.parse(event);
          std::cout << "Processed event: " << toString(eventPtr->type()) << std::endl;
        } catch (const std::exception& e) {
          std::cerr << "Error processing event: " << e.what() << std::endl;
        }
        break;
      default:
        std::cerr << "Unknown event type: " << event << std::endl;
        break;
    }
}

} // namespace Exchange