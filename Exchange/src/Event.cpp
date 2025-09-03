#include "Event.h"
#include "EventParser.h"
#include <boost/algorithm/string.hpp>

namespace Exchange {



Event::Event(EventType type) : type_(type) {}
Event::~Event() = default;

EventType Event::type() const {
  return type_;
}

OrderEvent::OrderEvent(EventType type, const std::string& userId, OrderIdType clientOrderId, const std::string& symbol) noexcept
  : Event(type), userId_(userId), clientOrderId_(clientOrderId), symbol_(symbol) {}


const std::string& OrderEvent::userId() const {
  return userId_;
}

const OrderIdType& OrderEvent::clientOrderId() const {
  return clientOrderId_;
}

const std::string& OrderEvent::symbol() const {
  return symbol_;
}

NewOrderEvent::NewOrderEvent(const std::string& userId, OrderIdType clientOrderId, 
  const std::string& symbol, QuantityType quantity, Side side, Type type, 
  PriceType price) noexcept
  : OrderEvent(EventType::NewOrder, userId, clientOrderId, symbol), quantity_(quantity), side_(side), type_(type), price_(price) {}



CancelOrderEvent::CancelOrderEvent(const std::string& userId, OrderIdType clientOrderId, const std::string& symbol, OrderIdType origOrderId) noexcept 
    :  OrderEvent(EventType::CancelOrder, userId, clientOrderId, symbol), origOrderId_(origOrderId) {}


TopOfBookEvent::TopOfBookEvent(const std::string& userId, OrderIdType clientOrderId, const std::string& symbol) noexcept 
  : OrderEvent(EventType::TopOfBook, userId, clientOrderId, symbol) {}

QuitEvent::QuitEvent() noexcept : Event(EventType::Quit) {}



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