#include "Order.h"
#include "Event.h"
#include <cassert>

namespace Exchange {

Order::Order(const NewOrderEvent& event, SequenceNumber sequenceNumber) noexcept
  : userId_(event.userId()), clientOrderId_(event.clientOrderId()), symbol_(event.symbol()),
    quantity_(event.quantity()), openQuantity_(event.quantity()), side_(event.side()), type_(event.type()), price_(event.price()),
    timestamp_(event.timestamp()), sequenceNumber_(sequenceNumber) {
}

Order::Order(const NewOrderEvent& event, SequenceNumber sequenceNumber, Quantity filledQuantity) noexcept
  : userId_(event.userId()), clientOrderId_(event.clientOrderId()), symbol_(event.symbol()),
    quantity_(event.quantity()), side_(event.side()), type_(event.type()), price_(event.price()),
    timestamp_(event.timestamp()), sequenceNumber_(sequenceNumber) {

  assert(filledQuantity <= quantity_);

  openQuantity_ = quantity_ - filledQuantity;
  state_ = openQuantity_ == 0 ? OrderState::Filled : OrderState::PartiallyFilled;
}

bool Order::isValid() const {
  return clientOrderId_ != INVALID_ORDER_ID;
}

bool Order::isActive() const {
  return state_ == OrderState::New || state_ == OrderState::PartiallyFilled;
}

Quantity Order::fill(Quantity qty) {
  if (!isActive() || qty <= 0) return 0;
  const Quantity d = std::min(openQuantity_, qty);
  openQuantity_ -= d;
  state_ = (openQuantity_ == 0) ? OrderState::Filled : OrderState::PartiallyFilled;
  return d;
}

void Order::cancel() {
  state_ = OrderState::Cancelled;
}

} // namespace Exchange