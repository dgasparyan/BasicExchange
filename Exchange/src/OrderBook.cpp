#include "OrderBook.h"
#include "Event.h"
#include <iostream>

namespace Exchange {

IOrderBook::~IOrderBook() = default;

void OrderBook::submitNewOrder(const NewOrderEvent& event) {
  std::cout << "OrderBook::submitNewOrder: " << event.symbol() << std::endl;
  // TODO: Implement
}

void OrderBook::submitCancelOrder(const CancelOrderEvent& event) {
  std::cout << "OrderBook::submitCancelOrder: " << event.symbol() << std::endl;
  // TODO: Implement
}

void OrderBook::submitTopOfBook(const TopOfBookEvent& event) {
  std::cout << "OrderBook::submitTopOfBook: " << event.symbol() << std::endl;
  // TODO: Implement
}

} // namespace Exchange 