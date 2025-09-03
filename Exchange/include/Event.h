#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <variant>
#include "OrderUtils.h"

namespace Exchange {

enum class EventType {
    NewOrder,
    CancelOrder,
    TopOfBook,

    Quit,

    Invalid
};

template <class Derived>
class OrderEvent {
  public:
    OrderEvent(UserIdType userId, OrderIdType clientOrderId, SymbolType symbol) noexcept 
      : userId_(userId), clientOrderId_(clientOrderId), symbol_(symbol) {}

    EventType eventType() const noexcept
    {
      auto& self = static_cast<const Derived&>(*this);
      return self.eventType();
    }

    UserIdType userId() const {
      return userId_;
    }
    OrderIdType clientOrderId() const {
      return clientOrderId_;
    }
    SymbolType symbol() const {
      return symbol_;
    }

  public:
    UserIdType userId_ {INVALID_USER_ID};
    OrderIdType clientOrderId_ {};
    SymbolType symbol_ {};
};

class NewOrderEvent : public OrderEvent<NewOrderEvent> {
  public:
    NewOrderEvent(UserIdType userId, OrderIdType clientOrderId, SymbolType symbol, QuantityType quantity, Side side, 
                  Type type, PriceType price = INVALID_PRICE) noexcept;

    EventType eventType() const noexcept {
      return EventType::NewOrder;
    }

  public:
      QuantityType quantity_ {INVALID_QUANTITY};
      Side side_ {Side::Invalid};
      Type type_ {Type::Invalid};
      PriceType price_ {INVALID_PRICE};
};

class CancelOrderEvent : public OrderEvent<CancelOrderEvent> {
  public:
    CancelOrderEvent(UserIdType userId, OrderIdType clientOrderId, SymbolType symbol, OrderIdType origOrderId) noexcept;

    EventType eventType() const noexcept {
      return EventType::CancelOrder;
    }

  public:
    OrderIdType origOrderId_ {};
};

class TopOfBookEvent : public OrderEvent<TopOfBookEvent> {
  public:
    TopOfBookEvent(UserIdType userId, OrderIdType clientOrderId, SymbolType symbol) noexcept;

    EventType eventType() const noexcept {
      return EventType::TopOfBook;
    }

};

class QuitEvent {
  public:
    EventType eventType() const noexcept {
      return EventType::Quit;
    }

};


using Event = std::variant<NewOrderEvent, CancelOrderEvent, TopOfBookEvent, QuitEvent>;

static_assert(std::is_trivially_copyable_v<Event>, "Event must be trivially copyable so we can use boost lockfree queues");

EventType toEventType(std::string_view eventType);
std::string toString(EventType eventType);


} // namespace Exchange

#endif // EVENT_H 