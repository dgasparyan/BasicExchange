#ifndef EVENT_H
#define EVENT_H

#include <string>

enum class EventType {
    NewOrder,
    CancelOrder,
    TopOfBook,

    Quit,



    Invalid
};

EventType toEventType(const std::string& eventType);
std::string toString(EventType eventType);






#endif // EVENT_H 