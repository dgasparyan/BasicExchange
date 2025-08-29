#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <string>
#include <functional>
#include <memory>

class SubscriptionHandle {
public:
  virtual ~SubscriptionHandle() = 0;
};

class EventQueue {
public:
  using MessageCallback = std::function<void(const std::string&)>; // (message)

  virtual std::unique_ptr<SubscriptionHandle> subscribe(MessageCallback callback) = 0;
  virtual ~EventQueue() = 0;

};


#endif // EVENT_QUEUE_H