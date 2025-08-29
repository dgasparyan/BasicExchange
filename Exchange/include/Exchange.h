#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <string>

class EventQueue;

class Exchange {
public:
    Exchange(EventQueue& eventQueue);
    ~Exchange();

private:
    void processEvent(const std::string& event);

private:
    EventQueue& eventQueue_;
};

#endif // EXCHANGE_H