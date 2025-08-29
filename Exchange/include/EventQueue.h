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

  template<class F, class... Args >
  // requires std::is_invocable_v<F&, Args..., const std::string&>
  requires std::invocable<F&, Args..., const std::string&>
  [[nodiscard]] std::unique_ptr<SubscriptionHandle>  subscribe_with(F&& f, Args&&... args) {
    MessageCallback cb = [g = std::forward<F>(f),
                          ...b = std::forward<Args>(args)](const std::string& msg) mutable {
      std::invoke(g, b..., msg);
    };
    return subscribe(std::move(cb));
  }

  [[nodiscard]] std::unique_ptr<SubscriptionHandle>  subscribe_with(MessageCallback callback) {
    return subscribe(std::move(callback));
  }

};


#endif // EVENT_QUEUE_H