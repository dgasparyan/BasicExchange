#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "Event.h"
#include "Order.h"
#include "ReportUtils.h"
#include <boost/multi_index_container.hpp>   // <-- the big one (not just the fwd)
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>     // for const_mem_fun

#include <functional>                        // std::less, std::greater
#include <vector>
#include <iostream>

namespace Exchange {

class IOrderBook {
public:
    virtual ~IOrderBook() = 0;


// TODO: change to rvalue references, we always move these anyway
    virtual bool submitNewOrder(const NewOrderEvent& event) = 0;
    virtual bool submitCancelOrder(const CancelOrderEvent& event) = 0;
    virtual void submitTopOfBook(const TopOfBookEvent& event) = 0;
};

template <typename ReportSink>
class OrderBook : public IOrderBook {
public:

    // TODO: Whole order book creation needs a bit of fixing.
    explicit OrderBook(ReportSink& reportSink) : reportSink_(reportSink) {}


    bool submitNewOrder(const NewOrderEvent& event) override;
    bool submitCancelOrder(const CancelOrderEvent& event) override;
    void submitTopOfBook(const TopOfBookEvent& event) override;

private:
  ReportSink& reportSink_;

  struct by_price_time_seq {};
  struct by_order_id;           // (userId, clientOrderId) or just clientOrderId

  // asks sorted with lowest price first, then timestamp, then sequence number(as a tie breaker)
  using AskBook = boost::multi_index::multi_index_container<
    Order,
    boost::multi_index::indexed_by<
      boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<by_price_time_seq>,
        boost::multi_index::composite_key<
          Order,
          boost::multi_index::const_mem_fun<Order, Price, &Order::price>,
          boost::multi_index::const_mem_fun<Order, Timestamp, &Order::timestamp>,
          boost::multi_index::const_mem_fun<Order, SequenceNumber, &Order::sequenceNumber>
        >,
        boost::multi_index::composite_key_compare<
          std::less<Price>, std::less<Timestamp>, std::less<SequenceNumber>
        >
      >,
      // 2nd index for fast cancelation
      boost::multi_index::hashed_unique<
      boost::multi_index::tag<by_order_id>,
        boost::multi_index::const_mem_fun<Order, OrderId, &Order::clientOrderId>
      >
    >
  >;

  // bids sorted with highest price first, then timestamp, then sequence number(as a tie breaker)
  using BidBook = boost::multi_index::multi_index_container<
    Order,
    boost::multi_index::indexed_by<
      boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<by_price_time_seq>,
        boost::multi_index::composite_key<
          Order,
          boost::multi_index::const_mem_fun<Order, Price, &Order::price>,
          boost::multi_index::const_mem_fun<Order, Timestamp, &Order::timestamp>,
          boost::multi_index::const_mem_fun<Order, SequenceNumber, &Order::sequenceNumber>
        >,
        boost::multi_index::composite_key_compare<
          std::greater<Price>, std::less<Timestamp>, std::less<SequenceNumber>
        >
      >,
      boost::multi_index::hashed_unique<
        boost::multi_index::tag<by_order_id>,
        boost::multi_index::const_mem_fun<Order, OrderId, &Order::clientOrderId>
      >
    >
  >;

  AskBook askBook_;
  BidBook bidBook_;

  uint64_t nextSequenceNumber();

  bool isAggressive(const NewOrderEvent& event, auto& container, auto cmpFunc);
  void handleAggressiveOrder(const NewOrderEvent& event, auto& sameSideContainer, auto& opposideSideBook, auto cmpFunc);
  bool handleNewOrder(const NewOrderEvent& event, auto& sameSideBook, auto& oppositeSideBook, auto cmpFunc);

  void reportFills(PFillVec&& fills);
  void reportNewOrderCanceled(const NewOrderEvent& event, Quantity filledQuantity);
  void reportOrderCanceled(const Order& order);

};

template <typename ReportSink>
bool OrderBook<ReportSink>::submitNewOrder(const NewOrderEvent& event) {

  // buy crosses asks: market always crosses; otherwise limit >= best ask
  auto crossesBuy = [](const NewOrderEvent& ev, Price bestAsk) noexcept {
    return ev.type() == Type::Market || ev.price() >= bestAsk;
  };

  // sell crosses bids: market always crosses; otherwise limit <= best bid
  auto crossesSell = [](const NewOrderEvent& ev, Price bestBid) noexcept {
    return ev.type() == Type::Market || ev.price() <= bestBid;
  };
  if (event.side() == Side::Buy) {
    return handleNewOrder(event, bidBook_, askBook_, crossesBuy);
  } else if (event.side() == Side::Sell) {
    return handleNewOrder(event, askBook_, bidBook_, crossesSell);
  } else {
    std::cout << "OrderBook::submitNewOrder: Invalid side" << std::endl;
    // throw an exception once we are doing exception handling properly
    return false;
  }
}

template <typename ReportSink>
bool OrderBook<ReportSink>::submitCancelOrder(const CancelOrderEvent& event) {
  auto cancelOrder = [this](auto& book, auto orderId) {
    auto& container = book.template get<by_order_id>();
    if (auto it = container.find(orderId); it != container.end()) {
      Order snapshot = *it;
      container.erase(it);
      snapshot.cancel();
      reportOrderCanceled(snapshot);
      return true;
    }
    return false;
  };
  const auto id = event.origOrderId();
  bool onBid = cancelOrder(bidBook_, id);
  bool onAsk = !onBid && cancelOrder(askBook_, id);
  assert(!(onBid && onAsk) && "Order ID present on both sides!");
  return onBid || onAsk;
}

template <typename ReportSink>
void OrderBook<ReportSink>::submitTopOfBook(const TopOfBookEvent& event) {
  reportSink_.submitTopOfBook(TopOfBookReport{
              bidBook_.empty() ? Order() : *bidBook_.begin(), 
              askBook_.empty() ? Order() : *askBook_.begin()});
}



template <typename ReportSink>
uint64_t OrderBook<ReportSink>::nextSequenceNumber() {
  static uint64_t sequenceNumber = 0;
  return sequenceNumber++;
}

template <typename ReportSink>
bool OrderBook<ReportSink>::isAggressive(const NewOrderEvent& event, auto& container, auto cmpFunc) {
  auto best = container.begin();
  return best != container.end() && cmpFunc(event, best->price());
}

template <typename ReportSink>
bool OrderBook<ReportSink>::handleNewOrder(const NewOrderEvent& event, auto& sameSideBook, auto& oppositeSideBook, auto cmpFunc) {
  auto& sameSideContainer = sameSideBook.template get<by_price_time_seq>();
  auto& oppositeSideContainer = oppositeSideBook.template get<by_price_time_seq>();

  if (isAggressive(event, oppositeSideContainer, cmpFunc)) { 
    handleAggressiveOrder(event, sameSideContainer, oppositeSideBook, cmpFunc);
    return true;
  } else {
    switch (event.type()) {
      case Type::Market:
        {
          // TODO: use the new cpp20 formatting stuff to add the ide and whatnot here
          assert(oppositeSideContainer.empty() && "Have opposite side orders but didn't fill a market order, something is wrong!");
          // if market then there were no opposite side orders, just cancel. 
          // cancel the order, it's FILL_OR_KILL (Fill and kill too)
          reportNewOrderCanceled(event, 0);
          return true;
        }
        break;
      case Type::Limit:
        {
          return sameSideContainer.emplace(event, nextSequenceNumber()).second;
        }
        break;
      default:
        assert(false && "OrderBook::processBuyOrder: Invalid type");
        break;
    }
  }

  return false;
}

template <typename ReportSink>
void OrderBook<ReportSink>::handleAggressiveOrder(const NewOrderEvent& event, auto& sameSideContainer, auto& opposideSideBook, auto cmpFunc) {
  auto& oppositeSideContainer = opposideSideBook.template get<by_price_time_seq>();

  PFillVec fills;
  Quantity filledQuantity = 0;
  auto it = oppositeSideContainer.begin(); 
  while (filledQuantity < event.quantity() && it != oppositeSideContainer.end() && cmpFunc(event, it->price())) {
    auto leaveQuantity = event.quantity() - filledQuantity;

    Quantity filled = 0;
    bool modified = oppositeSideContainer.modify(it, [&](Order& order) {
      filled = order.fill(leaveQuantity);
    });

    assert(modified);

    fills.emplace_back(it->clientOrderId(), filled);
    fills.emplace_back(event.clientOrderId(), filled);
    filledQuantity += filled;

    it = it->state() == OrderState::Filled ? oppositeSideContainer.erase(it) : std::next(it);
  }

  reportFills(std::move(fills));

  if (filledQuantity != event.quantity()) {
    // no more fills so canceling the rest of the order (FILL&KILL)
    if (it == oppositeSideContainer.end()) {
      reportNewOrderCanceled(event, filledQuantity);
    } else {
      sameSideContainer.emplace(event, nextSequenceNumber(), filledQuantity).second;
    }
  }
}

template <typename ReportSink>
void OrderBook<ReportSink>::reportNewOrderCanceled(const NewOrderEvent& event, Quantity filledQuantity) {
  reportSink_.submitCanceledOrder(CanceledOrderReport{event.clientOrderId(), event.quantity() - filledQuantity, CancelReason::Fill_And_Kill});
}

template <typename ReportSink>
void OrderBook<ReportSink>::reportOrderCanceled(const Order& order) {
  reportSink_.submitCanceledOrder(CanceledOrderReport{order.clientOrderId(), order.openQuantity(), CancelReason::User_Canceled});
}

template <typename ReportSink>
void OrderBook<ReportSink>::reportFills(PFillVec&& fills) {
  reportSink_.submitFills(std::move(fills));
}


} // namespace Exchange

#endif // ORDER_BOOK_H 