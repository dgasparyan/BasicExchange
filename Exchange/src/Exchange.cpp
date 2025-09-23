#include "Exchange.h"

#include <iostream>

#include "Event.h"
#include "EventParser.h"

namespace Exchange {


Exchange::Exchange(EventQueue& eventQueue, EventParser& eventParser, IOrderBookManager& orderBookManager) : orderBookManager_(orderBookManager), eventParser_(eventParser), eventQueue_(eventQueue){
}

Exchange::~Exchange() {
  requestStop();
  handleStop();
}

void Exchange::start() {
  eventQueueSubscription_ = eventQueue_.subscribeWith(&Exchange::processEvent, this);
  while (!stopRequested_.load(std::memory_order_acquire)) {
    stopRequested_.wait(false, std::memory_order_acquire);
  }
  handleStop();
}

void Exchange::requestStop() {
  stopRequested_.store(true, std::memory_order_release);
  stopRequested_.notify_all();
}

void Exchange::handleStop() {
  eventQueueSubscription_.reset();
  orderBookManager_.stop();
}

void Exchange::processEvent(const std::string& eventStr) noexcept {
  try {
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
    } catch (const std::exception& e) {
      std::cerr << "Error processing event: " << e.what() << std::endl;
    }
}

} // namespace Exchange