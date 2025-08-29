#ifndef UDP_LISTENER_H
#define UDP_LISTENER_H

#include <string>
#include <functional>
#include <thread>
#include <atomic>

#include "EventQueue.h"

namespace Exchange {

class UDPListener;

class UdpSubscriptionHandle : public SubscriptionHandle {
public:
  UdpSubscriptionHandle(UDPListener* listener, int handle);
  ~UdpSubscriptionHandle();

  UdpSubscriptionHandle(const UdpSubscriptionHandle&) = delete;
  UdpSubscriptionHandle& operator=(const UdpSubscriptionHandle&) = delete;

  UdpSubscriptionHandle(UdpSubscriptionHandle&& other) = default;
  UdpSubscriptionHandle& operator=(UdpSubscriptionHandle&& other)  = default;

private:
  UDPListener* listener_ {nullptr};
  int handle_ {-1};
};

class UDPListener : public EventQueue {
public:
   
    explicit UDPListener(int port);
    ~UDPListener();
    
    [[nodiscard]] std::unique_ptr<SubscriptionHandle> subscribe(MessageCallback callback) override;
    
private:
    void bindToPort(int port);
    bool startListening();
    void stopListening();
    void listenLoop();

    friend class UdpSubscriptionHandle;
    bool unsubscribe(int handle);

private:
    int socketFd_;
    int port_;

    std::mutex cbMutex_;
    std::unordered_map<int, MessageCallback> callbacks_;

    std::thread listenerThread_;
};

} // namespace Exchange

#endif // UDP_LISTENER_H 