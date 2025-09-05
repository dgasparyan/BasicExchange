#include "Event.h"
#include "EventParser.h"
#include <boost/algorithm/string.hpp>

namespace Exchange {


NewOrderEvent::NewOrderEvent(UserId userId, OrderId clientOrderId, 
  Symbol symbol, Quantity quantity, Side side, Type type, 
  Price price) noexcept
  : OrderEvent<NewOrderEvent>(userId, clientOrderId, symbol), quantity_(quantity), side_(side), type_(type), price_(price) {}



CancelOrderEvent::CancelOrderEvent(UserId userId, OrderId clientOrderId, Symbol symbol, OrderId origOrderId) noexcept 
    :  OrderEvent<CancelOrderEvent>(userId, clientOrderId, symbol), origOrderId_(origOrderId) {}


TopOfBookEvent::TopOfBookEvent(UserId userId, OrderId clientOrderId, Symbol symbol) noexcept 
  : OrderEvent<TopOfBookEvent>(userId, clientOrderId, symbol) {}


EventType toEventType(std::string_view eventType) {
  auto upperEvent = trimAndUpperCopy(eventType);
  
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