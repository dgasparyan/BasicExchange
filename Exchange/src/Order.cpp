#include "Order.h"
#include "Event.h"

namespace Exchange {

Order::Order(const NewOrderEvent& event, SequenceNumber sequenceNumber) noexcept
  : userId_(event.userId()), clientOrderId_(event.clientOrderId()), symbol_(event.symbol()),
    openQuantity_(event.quantity()), side_(event.side()), type_(event.type()), price_(event.price()),
    sequenceNumber_(sequenceNumber) {
}

} // namespace Exchange