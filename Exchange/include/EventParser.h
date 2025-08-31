#ifndef EVENT_PARSER_H
#define EVENT_PARSER_H

#include <string>
#include <memory>
#include "Event.h"
#include "OrderUtils.h"
#include "CommonUtils.h"


namespace Exchange {

class EventParser {
public:

  virtual ~EventParser() = default;

  virtual EventType getEventType(std::string_view event) const = 0;
  virtual std::unique_ptr<Event> parse(std::string_view event) const = 0;
};


class CsvEventParser : public EventParser {
public:

    EventType getEventType(std::string_view event) const override; 
    // parses a csv in the format:[D, UserID, ClinetOrderId, Symbol, Quantity, Side, Type, [Price]]
    std::unique_ptr<Event> parse(std::string_view event) const override;

  /*
      Could definitely just using a "normal" const std::vector<std::string>& argument here but
      good place to play around with ranges and whatnot so ...  
  */
  private:
    template<std::ranges::input_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, std::string>
    std::unique_ptr<NewOrderEvent> createNewOrderEvent(const Range& r) const {
      auto it = std::begin(r);
      if (toEventType(*it) != EventType::NewOrder) {
        throw std::runtime_error("Invalid event type: " + std::string(*it));
      }
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Order type: ");
      }
      auto userId = trimCopy(*it);
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the UserID: ");
      }
      auto clientOrderId = std::stoi(trimCopy(*it));
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the ClientOrderId: ");
      }
      auto symbol = trimCopy(*it);
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Symbol: ");
      }
      auto quantity = std::stoi(trimCopy(*it));
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Quantity: ");
      }
      auto side = toSide(*it);
      if (side == Side::Invalid) {
        throw std::runtime_error("Invalid side: " + std::string(*it));
      }
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Side: ");
      }
      auto type = toType(*it);
      if (type == Type::Invalid) {
        throw std::runtime_error("Invalid type: " + std::string(*it));
      }
      PriceType price = INVALID_PRICE;
      if (type == Type::Limit) {
        if (++it == std::end(r)) {
          throw std::runtime_error("Not enough tokens in event: Price missing for Limit order: ");
        }
        price = std::stod(trimCopy(*it));
      }

      return std::make_unique<NewOrderEvent>(userId, clientOrderId, symbol, quantity, side, type, price);
    }

    template<std::ranges::input_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, std::string>
    std::unique_ptr<CancelOrderEvent> createCancelOrderEvent(const Range& r) const {
      auto it = std::begin(r);
      if (toEventType(*it) != EventType::CancelOrder) {
        throw std::runtime_error("Invalid event type: " + std::string(*it));
      }
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Order type");
      }
      auto userId = trimCopy(*it);
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the UserID");
      }
      auto clientOrderId = std::stoi(trimCopy(*it));
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the ClientOrderId");
      }
      auto symbol = trimCopy(*it);
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Symbol");
      }
      auto origOrderId = std::stoi(trimCopy(*it));

      return std::make_unique<CancelOrderEvent>(userId, clientOrderId, symbol, origOrderId);
    }

    template<std::ranges::input_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, std::string>
    std::unique_ptr<TopOfBookEvent> createTopOfBookEvent(const Range& r) const {
      auto it = std::begin(r);
      if (toEventType(*it) != EventType::TopOfBook) {
        throw std::runtime_error("Invalid event type: " + std::string(*it));
      }
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Order type");
      }
      auto userId = trimCopy(*it);
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the UserID");
      }
      auto clientOrderId = std::stoi(trimCopy(*it));
      if (++it == std::end(r)) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the ClientOrderId");
      }
      auto symbol = trimCopy(*it);
      
      return std::make_unique<TopOfBookEvent>(userId, clientOrderId, symbol);
    }

    template<std::ranges::input_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, std::string>
    std::unique_ptr<QuitEvent> createQuitEvent(const Range& r) const {
      auto it = std::begin(r);
      if (toEventType(*it) != EventType::Quit) {
        throw std::runtime_error("Invalid event type: " + std::string(*it));
      }
      return std::make_unique<QuitEvent>();
    }

};

} // namespace Exchange

#endif // EVENT_PARSER_H 