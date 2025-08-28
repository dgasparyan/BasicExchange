#ifndef UDP_LISTENER_H
#define UDP_LISTENER_H

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class UDPListener {
public:
    using MessageCallback = std::function<void(const std::string&, const std::string&)>; // (message, sender_ip)
    
    UDPListener(int port);
    ~UDPListener();
    
    bool startListening(MessageCallback callback);
    
    void stopListening();
    
    bool isRunning() const { return running_; }
    
    std::string getLastError() const { return lastError_; }

private:
    int socketFd_;
    std::thread listenerThread_;
    std::atomic<bool> running_;
    std::string lastError_;
    
    void listenLoop(MessageCallback callback);
};

#endif // UDP_LISTENER_H 