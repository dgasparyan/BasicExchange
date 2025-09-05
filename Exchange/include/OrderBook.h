#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "Event.h"

namespace Exchange {

class IOrderBook {
public:
    virtual ~IOrderBook() = 0;

    virtual void submitNewOrder(const NewOrderEvent& event) = 0;
    virtual void submitCancelOrder(const CancelOrderEvent& event) = 0;
    virtual void submitTopOfBook(const TopOfBookEvent& event) = 0;
};

class OrderBook : public IOrderBook {
public:
    void submitNewOrder(const NewOrderEvent& event) override;
    void submitCancelOrder(const CancelOrderEvent& event) override;
    void submitTopOfBook(const TopOfBookEvent& event) override;

  private:
    // using Container = boost::multi_index::multi_index_container<

};

} // namespace Exchange

#endif // ORDER_BOOK_H 