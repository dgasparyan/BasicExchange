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

    TimestampType timestamp() const noexcept {
      return timestamp_;
    }

  public:
    UserIdType userId_ {INVALID_USER_ID};
    OrderIdType clientOrderId_ {};
    SymbolType symbol_ {};

    TimestampType timestamp_ {std::chrono::steady_clock::now()};
};

class NewOrderEvent : public OrderEvent<NewOrderEvent> {
  public:
    NewOrderEvent(UserIdType userId, OrderIdType clientOrderId, SymbolType symbol, QuantityType quantity, Side side, 
                  Type type, PriceType price = INVALID_PRICE) noexcept;

    EventType eventType() const noexcept {
      return EventType::NewOrder;
    }

    QuantityType quantity() const {
      return quantity_;
    }
    Side side() const {
      return side_;
    }
    Type type() const {
      return type_;
    }
    PriceType price() const {
      return price_;
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


using EventVariant = std::variant<std::monostate, NewOrderEvent, CancelOrderEvent, TopOfBookEvent, QuitEvent>;

template <class T>
concept HasSymbol = requires (const T& event) {
  { event.symbol() } -> std::convertible_to<SymbolType>;
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

  SymbolType symbol() const {
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

  // SymbolType symbol_ {INVALID_SYMBOL};
  EventVariant data_ {};
};

static_assert(std::is_trivially_copyable_v<Event>, "Event must be trivially copyable so we can use boost lockfree queues");

EventType toEventType(std::string_view eventType);
std::string toString(EventType eventType);


} // namespace Exchange

#endif // EVENT_H 