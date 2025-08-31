#ifndef ORDER_BOOK_MANAGER_H
#define ORDER_BOOK_MANAGER_H

#include <map>

// #include "OrderBook.h"
#include "Event.h"

namespace Exchange {

  class IOrderBookManager {
  public:
    virtual ~IOrderBookManager() = 0;

  };

class OrderBookManager : public IOrderBookManager {
public:
    OrderBookManager();


  private:
    // std::unordered_map<std::string, OrderBook> orderBooks_;
};

} // namespace Exchange

#endif // ORDER_BOOK_MANAGER_H 