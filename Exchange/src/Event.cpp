#include "Event.h"
#include <boost/algorithm/string.hpp>

namespace Exchange {

EventType toEventType(const std::string& eventType) {
    std::string trimmedEvent = eventType;
    boost::algorithm::trim(trimmedEvent);
    std::string upperEvent = boost::algorithm::to_upper_copy(trimmedEvent);
  
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