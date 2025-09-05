#ifndef EVENT_H
#define EVENT_H

#include <variant>
#include <concepts>
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

    OrderEvent(UserId userId, OrderId clientOrderId, Symbol symbol) noexcept 
      : userId_(userId), clientOrderId_(clientOrderId), symbol_(symbol) {}

    EventType eventType() const noexcept
    {
      auto& self = static_cast<const Derived&>(*this);
      return self.eventType();
    }

    UserId userId() const {
      return userId_;
    }
    OrderId clientOrderId() const {
      return clientOrderId_;
    }
    Symbol symbol() const {
      return symbol_;
    }

    Timestamp timestamp() const noexcept {
      return timestamp_;
    }

  public:
    UserId userId_ {INVALID_USER_ID};
    OrderId clientOrderId_ {};
    Symbol symbol_ {};

    Timestamp timestamp_ {std::chrono::steady_clock::now()};
};

class NewOrderEvent : public OrderEvent<NewOrderEvent> {
  public:
    NewOrderEvent(UserId userId, OrderId clientOrderId, Symbol symbol, Quantity quantity, Side side, 
                  Type type, Price price = INVALID_PRICE) noexcept;

    EventType eventType() const noexcept {
      return EventType::NewOrder;
    }

    Quantity quantity() const {
      return quantity_;
    }
    Side side() const {
      return side_;
    }
    Type type() const {
      return type_;
    }
    Price price() const {
      return price_;
    }

  public:
      Quantity quantity_ {INVALID_QUANTITY};
      Side side_ {Side::Invalid};
      Type type_ {Type::Invalid};
      Price price_ {INVALID_PRICE};
};

class CancelOrderEvent : public OrderEvent<CancelOrderEvent> {
  public:
    CancelOrderEvent(UserId userId, OrderId clientOrderId, Symbol symbol, OrderId origOrderId) noexcept;

    EventType eventType() const noexcept {
      return EventType::CancelOrder;
    }

  public:
    OrderId origOrderId_ {};
};

class TopOfBookEvent : public OrderEvent<TopOfBookEvent> {
  public:
    TopOfBookEvent(UserId userId, OrderId clientOrderId, Symbol symbol) noexcept;

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


using EventVariant = std::variant<std::monostate, NewOrderEvent, CancelOrderEvent, TopOfBookEvent, QuitEvent>;

template <class T>
concept HasSymbol = requires (const T& event) {
      { event.symbol() } -> std::convertible_to<Symbol>;
};

template <class T>
concept AllowNoSymbol = 
    std::is_same_v<std::decay_t<T>, QuitEvent> 
    || std::is_same_v<std::decay_t<T>, std::monostate>;

struct Event {

  // Convenience: construct the variant arm by type
  template<class T, class... Args>
  explicit Event(std::in_place_type_t<T>, Args&&... args)
    : data_(std::in_place_type<T>, std::forward<Args>(args)...) {}
  
  Event() = default;

      Symbol symbol() const {
    auto visitor = [](auto&& event) {
      using T = std::decay_t<decltype(event)>;
      if constexpr (HasSymbol<T>) {
        return event.symbol();
      } else if constexpr (AllowNoSymbol<T>) {
        return INVALID_SYMBOL;
      } 
      else {
        static_assert(false, "Should either have a symbol or be declared as AllowNoSymbol");
      }
    };

    return std::visit(visitor, data_);
  }

      // Symbol symbol_ {INVALID_SYMBOL};
  EventVariant data_ {};
};

static_assert(std::is_trivially_copyable_v<Event>, "Event must be trivially copyable so we can use boost lockfree queues");

EventType toEventType(std::string_view eventType);
std::string toString(EventType eventType);


} // namespace Exchange

#endif // EVENT_H 