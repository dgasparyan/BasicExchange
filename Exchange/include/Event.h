#ifndef EVENT_H
#define EVENT_H

#include <string>
#include "OrderUtils.h"

namespace Exchange {

enum class EventType {
    NewOrder,
    CancelOrder,
    TopOfBook,

    Quit,



    Invalid
};


class Event {
  public:
    explicit Event(EventType type);
    virtual ~Event() = 0;

    virtual EventType type() const;

  private:
    EventType type_;
};

class NewOrderEvent : public Event {
  public:
    NewOrderEvent(const std::string& userId, OrderIdType clientOrderId, const std::string& symbol, QuantityType quantity, Side side, 
                  Type type, PriceType price = INVALID_PRICE) noexcept;

  public:
      std::string userId_ {};
      OrderIdType clientOrderId_ {};
      std::string symbol_ {};

      QuantityType quantity_ {INVALID_QUANTITY};
      Side side_ {Side::Invalid};
      Type type_ {Type::Invalid};
      PriceType price_ {INVALID_PRICE};
};

class CancelOrderEvent : public Event {
  public:
    CancelOrderEvent(const std::string& userId, OrderIdType origOrderId, const std::string& symbol) noexcept;

  public:
    std::string userId_ {};
    OrderIdType origOrderId_ {};
    std::string symbol_ {};
};
class TopOfBookEvent : public Event {
  public:
    TopOfBookEvent(const std::string& userId, const std::string& symbol) noexcept;

  public:
    std::string userId_ {};
    std::string symbol_ {};
};
class QuitEvent : public Event {
  public:
    QuitEvent() noexcept;

  public:
    std::string userId_ {};
};

template<EventType Type>
struct EventTraits {

};

template<>
struct EventTraits<EventType::NewOrder> {
  using EventType = NewOrderEvent;
  static constexpr const char* name = "NewOrder";
};

template<>
struct EventTraits<EventType::CancelOrder> {
  using EventType = CancelOrderEvent;
  static constexpr const char* name = "CancelOrder";
};

template<>
struct EventTraits<EventType::TopOfBook> {
  using EventType = TopOfBookEvent;
  static constexpr const char* name = "TopOfBook";
};

template<>
struct EventTraits<EventType::Quit> {
  using EventType = QuitEvent;
  static constexpr const char* name = "Quit";
};
// struct EventTraits<EventType::Invalid> {

EventType toEventType(const std::string& eventType);
std::string toString(EventType eventType);


} // namespace Exchange

#endif // EVENT_H 