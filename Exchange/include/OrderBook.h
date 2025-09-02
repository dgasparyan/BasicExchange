#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

namespace Exchange {

class IOrderBook {
public:
    virtual ~IOrderBook() = 0;
};

class OrderBook : public IOrderBook {
public:
};

} // namespace Exchange

#endif // ORDER_BOOK_H 