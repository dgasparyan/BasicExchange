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
    virtual void submitNewOrder(const NewOrderEvent& event) = 0;
    virtual void submitCancelOrder(const CancelOrderEvent& event) = 0;
    virtual void submitTopOfBook(const TopOfBookEvent& event) = 0;
};

template <typename ReportSink>
class OrderBook : public IOrderBook {
public:

    // TODO: Whole order book creation needs a bit of fixing.
    explicit OrderBook(ReportSink& reportSink) : reportSink_(reportSink) {}


    void submitNewOrder(const NewOrderEvent& event) override;
    void submitCancelOrder(const CancelOrderEvent& event) override;
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

  uint64_t nextSequenceNumber () {
    static uint64_t sequenceNumber = 0;
    return sequenceNumber++;
  }

  bool isAggressive(const NewOrderEvent& event, auto& container, auto cmpFunc) {
    auto best = container.begin();
    return best != container.end() && cmpFunc(event, best->price());
  }

  bool handleNewOrder(const NewOrderEvent& event, auto& sameSideBook, auto& oppositeSideBook, auto cmpFunc) {

      auto& sameSideContainer = sameSideBook.template get<by_price_time_seq>();
      auto& oppositeSideContainer = oppositeSideBook.template get<by_price_time_seq>();

      if (isAggressive(event, oppositeSideContainer, cmpFunc)) { 
        handleAggressiveOrder(event, sameSideContainer, oppositeSideBook, cmpFunc);
      } else {
        switch (event.type()) {
          case Type::Market:
            {
              // TODO: use the new cpp20 formatting stuff to add the ide and whatnot here
              assert(oppositeSideContainer.empty() && "Have opposite side orders but didn't fill a market order, something is wrong!");
              // if market then there were no opposite side orders, just cancel. 
              // cancel the order, it's FILL_OR_KILL (Fill and kill too)
              cancelNewOrderEvent(event, 0);
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

  // TODO: report properly
  using PFillData = std::pair<OrderId, Quantity>;
  using PFillVec = std::vector<PFillData>;

  void handleAggressiveOrder(const NewOrderEvent& event, auto& sameSideContainer, auto& opposideSideBook, auto cmpFunc) {
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
        cancelNewOrderEvent(event, filledQuantity);
      } else {
        sameSideContainer.emplace(event, nextSequenceNumber(), filledQuantity).second;
      }
    }
  }

  void reportFills(PFillVec&& fills) {
    reportSink_.submitFills(std::move(fills));
  }

    // if (filledQuantity < event.quantity()) {
      // cancel the order, it's FILL_OR_KILL (Fill and kill too)

  void cancelNewOrderEvent(const NewOrderEvent& event, Quantity filledQuantity);
};

template <typename ReportSink>
void OrderBook<ReportSink>::submitNewOrder(const NewOrderEvent& event) {

  // buy crosses asks: market always crosses; otherwise limit >= best ask
  auto crossesBuy = [](const NewOrderEvent& ev, Price bestAsk) noexcept {
    return ev.type() == Type::Market || ev.price() >= bestAsk;
  };

  // sell crosses bids: market always crosses; otherwise limit <= best bid
  auto crossesSell = [](const NewOrderEvent& ev, Price bestBid) noexcept {
    return ev.type() == Type::Market || ev.price() <= bestBid;
  };
  if (event.side() == Side::Buy) {
    handleNewOrder(event, bidBook_, askBook_, crossesBuy);
  } else if (event.side() == Side::Sell) {
    handleNewOrder(event, askBook_, bidBook_, crossesSell);
  } else {
    std::cout << "OrderBook::submitNewOrder: Invalid side" << std::endl;
    // throw an exception once we are doing exception handling properly
    return;
  }
}

template <typename ReportSink>
void OrderBook<ReportSink>::submitCancelOrder(const CancelOrderEvent& event) {
  // std::cout << "OrderBook::submitCancelOrder: " << event.symbol() << std::endl;
  // TODO: Implement
}

template <typename ReportSink>
void OrderBook<ReportSink>::submitTopOfBook(const TopOfBookEvent& event) {
  // std::cout << "OrderBook::submitTopOfBook: " << event.symbol() << std::endl;
  // TODO: Implement
}


template <typename ReportSink>
void OrderBook<ReportSink>::cancelNewOrderEvent(const NewOrderEvent& event, Quantity filledQuantity) {
  reportSink_.submitCanceledOrder(CanceledOrderReport{event.clientOrderId(), event.quantity() - filledQuantity, CancelReason::Fill_And_Kill});
}

} // namespace Exchange

#endif // ORDER_BOOK_H 