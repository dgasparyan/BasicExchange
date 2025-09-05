#ifndef EVENT_PARSER_H
#define EVENT_PARSER_H

#include <string>
#include <memory>
#include "Event.h"
#include "OrderUtils.h"
#include "CommonUtils.h"

#ifdef UNIT_TESTS
#  include <gtest/gtest_prod.h>
#  define EP_FRIEND_TEST(F, T) FRIEND_TEST(F, T);
#else
#  define EP_FRIEND_TEST(F, T)
#endif


namespace Exchange {

class EventParser {
public:

  virtual ~EventParser() = default;

  virtual EventType getEventType(std::string_view event) const = 0;
  virtual Event parse(std::string_view event) const = 0;
};


class CsvEventParser : public EventParser {
public:

    EventType getEventType(std::string_view event) const override; 
    // parses a csv in the format:[D, UserID, ClinetOrderId, Symbol, Quantity, Side, Type, [Price]]
    Event parse(std::string_view event) const override;


  /*
      Could definitely just using a "normal" const std::vector<std::string>& argument here but
      good place to play around with ranges and whatnot so ...  
  */
  private:

    template <class It, class Sentinel>
    requires std::input_iterator<It> &&
            std::sentinel_for<Sentinel, It> &&
            std::convertible_to<std::iter_reference_t<It>, std::string_view>
    Event createNewOrderEvent(It it, Sentinel last) const {
      // auto it = std::ranges::begin(r);
      if (toEventType(*it) != EventType::NewOrder) {
        throw std::runtime_error("Invalid event type: " + std::string(*it));
      }
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Order type: ");
      }
      auto userId = UserIdType(trimCopy(*it));
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the UserID: ");
      }
      auto clientOrderId = std::stoi(trimCopy(*it));
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the ClientOrderId: ");
      }
      auto symbol = SymbolType(trimCopy(*it));
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Symbol: ");
      }
      auto quantity = std::stoi(trimCopy(*it));
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Quantity: ");
      }
      auto side = toSide(*it);
      if (side == Side::Invalid) {
        throw std::runtime_error("Invalid side: " + std::string(*it));
      }
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Side: ");
      }
      auto type = toType(*it);
      if (type == Type::Invalid) {
        throw std::runtime_error("Invalid type: " + std::string(*it));
      }
      PriceType price = INVALID_PRICE;
      if (type == Type::Limit) {
        if (++it == last) {
          throw std::runtime_error("Not enough tokens in event: Price missing for Limit order: ");
        }
        price = toPrice(std::stod(trimCopy(*it)), TWO_DIGITS_PRICE_SPEC);
      }

      return Event{std::in_place_type<NewOrderEvent>, userId, clientOrderId, symbol, quantity, side, type, price};
    }

    Event
    createNewOrderEvent(std::initializer_list<std::string_view> il) const {
      return createNewOrderEvent(il.begin(), il.end());
  }

  template<std::ranges::input_range Range>
  // requires std::same_as<std::ranges::range_value_t<Range>, std::string>
  requires std::convertible_to<std::ranges::range_reference_t<Range>, std::string_view>
  Event createNewOrderEvent(Range&& r) const {
    return createNewOrderEvent(std::ranges::begin(r), std::ranges::end(r));
  }


  template <class It, class Sentinel>
  requires std::input_iterator<It> &&
          std::sentinel_for<Sentinel, It> &&
          std::convertible_to<std::iter_reference_t<It>, std::string_view>
    Event createCancelOrderEvent(It it, Sentinel last) const {
      if (toEventType(*it) != EventType::CancelOrder) {
        throw std::runtime_error("Invalid event type: " + std::string(*it));
      }
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Order type");
      }
      auto userId = UserIdType(trimCopy(*it));
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the UserID");
      }
      auto clientOrderId = std::stoi(trimCopy(*it));
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the ClientOrderId");
      }
      auto symbol = SymbolType(trimCopy(*it));
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Symbol");
      }
      auto origOrderId = std::stoi(trimCopy(*it));

      return Event{std::in_place_type<CancelOrderEvent>, userId, clientOrderId, symbol, origOrderId};
    }

    template<std::ranges::input_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, std::string>
    Event createCancelOrderEvent(Range&& r) const {
      return createCancelOrderEvent(std::ranges::begin(r), std::ranges::end(r));
    }

    Event
    createCancelOrderEvent(std::initializer_list<std::string_view> il) const {
      return createCancelOrderEvent(il.begin(), il.end());
    }

    template <class It, class Sentinel>
    requires std::input_iterator<It> &&
            std::sentinel_for<Sentinel, It> &&
            std::convertible_to<std::iter_reference_t<It>, std::string_view>
    Event createTopOfBookEvent(It it, Sentinel last) const {
      if (toEventType(*it) != EventType::TopOfBook) {
        throw std::runtime_error("Invalid event type: " + std::string(*it));
      }
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the Order type");
      }
      auto userId = UserIdType(trimCopy(*it));
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the UserID");
      }
      auto clientOrderId = std::stoi(trimCopy(*it));
      if (++it == last) {
        throw std::runtime_error("Not enough tokens in event: Only parsed up to the ClientOrderId");
      }
      auto symbol = SymbolType(trimCopy(*it));
      
      return Event{std::in_place_type<TopOfBookEvent>, userId, clientOrderId, symbol};
    }

    template<std::ranges::input_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, std::string>
    Event createTopOfBookEvent(Range&& r) const {
      return createTopOfBookEvent(std::ranges::begin(r), std::ranges::end(r));
    }

    Event
    createTopOfBookEvent(std::initializer_list<std::string_view> il) const {
      return createTopOfBookEvent(il.begin(), il.end());
    }

    template <class It, class Sentinel>
    requires std::input_iterator<It> &&
            std::sentinel_for<Sentinel, It> &&
            std::convertible_to<std::iter_reference_t<It>, std::string_view>
    Event createQuitEvent(It it, Sentinel) const {

      if (toEventType(*it) != EventType::Quit) {
        throw std::runtime_error("Invalid event type: " + std::string(*it));
      }
      return Event{std::in_place_type<QuitEvent>};
    }

    template<std::ranges::input_range Range>
    requires std::same_as<std::ranges::range_value_t<Range>, std::string>
    Event createQuitEvent(Range&& r) const {
      return createQuitEvent(std::ranges::begin(r), std::ranges::end(r));
    }

    Event
    createQuitEvent(std::initializer_list<std::string_view> il) const {
      return createQuitEvent(il.begin(), il.end());
    }



#ifdef UNIT_TESTS

    EP_FRIEND_TEST(EventParserTest, CreateNewOrderEvent_VariousRanges)
    EP_FRIEND_TEST(EventParserTest, CreateCancelOrderEvent_VariousRanges)
    EP_FRIEND_TEST(EventParserTest, CreateTopOfBookEvent_VariousRanges)
    EP_FRIEND_TEST(EventParserTest, CreateQuitEvent_VariousRanges)
#endif


};

} // namespace Exchange

#endif // EVENT_PARSER_H 