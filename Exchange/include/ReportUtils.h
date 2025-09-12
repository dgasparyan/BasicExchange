#ifndef REPORT_UTILS_H
#define REPORT_UTILS_H

#include <vector>
#include "Order.h"
#include "OrderUtils.h"
#include <format>

namespace Exchange {


  // TODO: report properly

struct ExecutionReport {
  ExecutionReport(Symbol symbol, OrderId orderId, OrderId otherOrderId, Quantity filledQuantity, Price price)
    : symbol_(symbol), orderId_(orderId), otherOrderId_(otherOrderId), filledQuantity_(filledQuantity), price_(price) {}
  
  Symbol symbol_;
  OrderId orderId_;
  OrderId otherOrderId_;
  
  Quantity filledQuantity_ {};
  Price price_ {};
};

using ExecutionReportCollection = std::vector<ExecutionReport>;


enum class CancelReason {
  Fill_And_Kill,
  User_Canceled,
  Other
};

struct OrderCanceledReport {
  Symbol symbol;
  OrderId orderId_ {INVALID_ORDER_ID};
  Quantity remainingQuantity_;
  CancelReason reason_;
};

struct SingleOrderReport {
  bool isValid() const { return orderId_ != INVALID_ORDER_ID; }

  OrderId orderId_ {INVALID_ORDER_ID};
  Price price_ {INVALID_PRICE};
  Quantity openQuantity_ {INVALID_QUANTITY};
};

struct TopOfBookReport {
  TopOfBookReport() = default;
  TopOfBookReport(Symbol symbol, const Order& bid, const Order& ask);

  bool isValid() const { return symbol_ != INVALID_SYMBOL; }

  Symbol symbol_ {INVALID_SYMBOL};
  SingleOrderReport bid_order_ {}; 
  SingleOrderReport ask_order_ {};
};

} // namespace Exchange

namespace  std {

  // ---------- ExecutionReport ----------

  template<>
  struct std::formatter<Exchange::ExecutionReport> : std::formatter<std::string_view> {
    // support passing format specifiers through, e.g. "{:>80}"
    template<class ParseContext>
    constexpr auto parse(ParseContext& ctx) { return std::formatter<std::string_view>::parse(ctx); }

    template<class FormatContext>
    auto format(const Exchange::ExecutionReport& r, FormatContext& ctx) const {
      auto s = std::format(
        "ExecutionReport{{symbol={}, orderId={}, otherOrderId={}, filledQuantity={}, price={:.2f}}}",
        r.symbol_, r.orderId_, r.otherOrderId_, r.filledQuantity_, r.price_);
      return std::formatter<std::string_view>::format(s, ctx);
    }
  };


// ---------- CancelReason ----------
template<>
struct formatter<Exchange::CancelReason, char> {
  formatter<string_view, char> base_;

  constexpr auto parse(basic_format_parse_context<char>& ctx) {
    return base_.parse(ctx);
  }

  template<class FC>
  auto format(Exchange::CancelReason r, FC& fc) const {
    std::string_view s = "Other";
    switch (r) {
      case Exchange::CancelReason::Fill_And_Kill: s = "Fill_And_Kill"; break;
      case Exchange::CancelReason::User_Canceled: s = "User_Canceled"; break;
      case Exchange::CancelReason::Other:         s = "Other"; break;
    }
    return base_.format(s, fc);
  }
};

// ---------- SingleOrderReport ----------
template<>
struct formatter<Exchange::SingleOrderReport, char> {
  formatter<string_view, char> base_;

  constexpr auto parse(basic_format_parse_context<char>& ctx) {
    return base_.parse(ctx);
  }

  template<class FC>
  auto format(const Exchange::SingleOrderReport& r, FC& fc) const {
    // Build a small string, then let base_ handle width/alignment.
    std::string tmp;
    std::format_to(std::back_inserter(tmp),
                   "SingleOrderReport{{orderId={}, price={}, openQty={}}}",
                   r.orderId_, r.price_, r.openQuantity_);
    return base_.format(std::string_view(tmp), fc);
  }
};

// ---------- OrderCanceledReport ----------
template<>
struct formatter<Exchange::OrderCanceledReport, char> {
  formatter<string_view, char> base_;

  constexpr auto parse(basic_format_parse_context<char>& ctx) {
    return base_.parse(ctx);
  }

  template<class FC>
  auto format(const Exchange::OrderCanceledReport& r, FC& fc) const {
    std::string tmp;
    std::format_to(std::back_inserter(tmp),
                   "OrderCanceledReport{{symbol={}, orderId={}, remaining={}, reason={}}}",
                   r.symbol, r.orderId_, r.remainingQuantity_, r.reason_);
    return base_.format(std::string_view(tmp), fc);
  }
};

// ---------- TopOfBookReport ----------
template<>
struct formatter<Exchange::TopOfBookReport, char> {
  formatter<string_view, char> base_;

  constexpr auto parse(basic_format_parse_context<char>& ctx) {
    return base_.parse(ctx);
  }

  template<class FC>
  auto format(const Exchange::TopOfBookReport& r, FC& fc) const {
    std::string tmp;
    std::format_to(std::back_inserter(tmp),
                   "TopOfBookReport{{symbol={}, bid={}, ask={}}}",
                   r.symbol_, r.bid_order_, r.ask_order_);
    return base_.format(std::string_view(tmp), fc);
  }
};


}


#endif // REPORT_UTILS_H