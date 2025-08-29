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
  // TODO: Start the worker thread
  eventQueueSubscription_ = eventQueue_.subscribe_with(&Exchange::processEvent, this);

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
    EventType eventType = toEventType(event);
    
    if (eventType == EventType::Quit) {
      stop();
    }
    else {
      try {
        auto eventPtr = eventParser_.parse(event);
        std::cout << "Processed event: " << toString(eventPtr->type()) << std::endl;
      } catch (const std::exception& e) {
        std::cerr << "Error processing event: " << e.what() << std::endl;
      }
    }

}

} // namespace Exchange