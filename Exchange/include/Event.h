#ifndef EVENT_H
#define EVENT_H

#include <string>

namespace Exchange {

enum class EventType {
    NewOrder,
    CancelOrder,
    TopOfBook,

    Quit,



    Invalid
};

EventType toEventType(const std::string& eventType);
std::string toString(EventType eventType);

} // namespace Exchange

#endif // EVENT_H 