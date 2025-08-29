#include "Event.h"
#include "EventParser.h"
#include <boost/algorithm/string.hpp>

namespace Exchange {



Event::Event(EventType type) : type_(type) {}
Event::~Event() = default;

EventType Event::type() const {
  return type_;
}

NewOrderEvent::NewOrderEvent(const std::string& userId, OrderIdType clientOrderId, 
  const std::string& symbol, QuantityType quantity, Side side, Type type, 
  PriceType price) noexcept : Event(EventType::NewOrder), userId_(userId), clientOrderId_(clientOrderId), 
  symbol_(symbol), quantity_(quantity), side_(side), type_(type), price_(price) {}



  CancelOrderEvent::CancelOrderEvent(const std::string& userId, OrderIdType origOrderId, const std::string& symbol) noexcept 
    : Event(EventType::CancelOrder), userId_(userId), origOrderId_(origOrderId), symbol_(symbol) {}
TopOfBookEvent::TopOfBookEvent(const std::string& userId, const std::string& symbol) noexcept 
  : Event(EventType::TopOfBook), userId_(userId), symbol_(symbol) {}
  
QuitEvent::QuitEvent() noexcept : Event(EventType::Quit) {}





EventType toEventType(const std::string& eventType) {
  std::string trimmedEvent = eventType;
  boost::algorithm::trim(trimmedEvent);
  std::string upperEvent = boost::algorithm::to_upper_copy(trimmedEvent);

  if (upperEvent == "D") {
      return EventType::NewOrder;
  } else if (upperEvent == "F") {
      return EventType::CancelOrder;
  } else if (upperEvent == "V") {
      return EventType::TopOfBook;
  } else if (upperEvent == "Q" || upperEvent == "QUIT") {
      return EventType::Quit;
  }
  
  return EventType::Invalid; // Default case
}

std::string toString(EventType eventType) {
  switch (eventType) {
      case EventType::NewOrder:
          return "NewOrder";
      case EventType::CancelOrder:
          return "CancelOrder";
      case EventType::TopOfBook:
          return "TopOfBook";
      case EventType::Quit:
          return "Quit";
      case EventType::Invalid:
          return "Invalid";
      default:
          return "Unknown";
  }
}


} // namespace Exchange