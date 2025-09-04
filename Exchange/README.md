Going to build a simple "Exchange". 

We'll process incoming orders from a UDP PORT ($PORT) in the following format:
# New order

D, UserID, ClinetOrderId, Symbol, Quantity, Side, Type, [Price]

Type: 
  - Market Order: [Market | 1]
  - Limit Order: [Limit | 2]

Side:
  - Buy: [Buy | 1]
  - Sell: [Sell | 2]

# Cancel Order

F, UserID, ClientOrderId, Symbol, OrigOrderId

# Top Of the Book

V, UserID, ClientOrderId, Symbol

==== 

Supported Order Types will be:
- Market, (assumed Fill AND kill if not enough liquidity)
- Limit, (support partial fills too)

====

Executions will be reported to the console as follows:



===

How to Build and run:

  - Dependencies:
    -- boost, with BOOST_ROOT set to your Boost directory (defaults to /opt/homebrew)




---

TODO:

- Teting after modernization

- switch to string_view(s) (maybe)

- Add ouw own Order/Event Ids, we don't want to rely on Client's

- Add a config and get a list of supported symbols from there, Change OrderBookManager to use that list and 
  create The orderBooks by cloning so we can test it better

  -- Decide on what to do with leftover events when stopping.

  -- better error/exception handling (like if the queue is full, or event processing throws etc)