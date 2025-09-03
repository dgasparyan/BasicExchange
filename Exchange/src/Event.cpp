#include "Event.h"
#include "EventParser.h"
#include <boost/algorithm/string.hpp>

namespace Exchange {


NewOrderEvent::NewOrderEvent(UserIdType userId, OrderIdType clientOrderId, 
  SymbolType symbol, QuantityType quantity, Side side, Type type, 
  PriceType price) noexcept
  : OrderEvent<NewOrderEvent>(userId, clientOrderId, symbol), quantity_(quantity), side_(side), type_(type), price_(price) {}



CancelOrderEvent::CancelOrderEvent(UserIdType userId, OrderIdType clientOrderId, SymbolType symbol, OrderIdType origOrderId) noexcept 
    :  OrderEvent<CancelOrderEvent>(userId, clientOrderId, symbol), origOrderId_(origOrderId) {}


TopOfBookEvent::TopOfBookEvent(UserIdType userId, OrderIdType clientOrderId, SymbolType symbol) noexcept 
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