#include "EventParser.h"
#include "Event.h"
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

namespace Exchange {

namespace {
  std::vector<std::string> parseCSVLine(std::string_view sv) {
    using It  = std::string_view::const_iterator;
    using Sep = boost::escaped_list_separator<char>;

    // Constructor: tokenizer<Separator, Iterator>(first, last, separator)
    boost::tokenizer<Sep, It> tok(sv.begin(), sv.end(), Sep('\\', ',', '"'));

    std::vector<std::string> out;
    out.assign(tok.begin(), tok.end());
    return out;
}

}


EventType CsvEventParser::getEventType(std::string_view event) const {
  auto trimmedUpper = trimAndUpperCopy(event.substr(0, event.find(',')));
  return toEventType(trimmedUpper);
}


std::unique_ptr<Event> CsvEventParser::parse(std::string_view event) const {
  auto tokens = parseCSVLine(event);

  if (tokens.size() < 1 ) {
    throw std::runtime_error("Not enough tokens in event: " + std::string(event));
  }
  EventType eventType = toEventType(tokens[0]);

  switch (eventType) {
    case EventType::NewOrder:
      return createNewOrderEvent(tokens);
    case EventType::CancelOrder:
      return createCancelOrderEvent(tokens);
    case EventType::TopOfBook:
      return createTopOfBookEvent(tokens);
    case EventType::Quit:
      return createQuitEvent(tokens);
    default:
      throw std::runtime_error("Invalid event type: " + std::string(tokens[0]));
  }
}


} // namespace Exchange 