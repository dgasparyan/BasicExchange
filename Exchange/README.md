Going to build a simple "Exchange". 

We'll process incoming orders from a UDP PORT ($PORT) in the following format:
# New order

D, UserID, ClinetOrderId, Symbol, Quantity, Type, Side, [Price]

Type: 
  - Market Order: [Market | 1]
  - Limit Order: [Limit | 2]

Side:
  - Buy: [B | 1]
  - Sell: [S | 2]

# Cancel Order

F, UserID, OrigOrderId, Symbol

# Top Of the Book

V, UserID, Symbol

==== 

Supported Order Types will be:
- Market, (assumed Fill AND kill if not enough liquidity)
- Limit, (support partial fills too)

====

Executions will be reported to the console as follows:



===
===

How to Build and run:

  - Dependencies:
    -- boost, with BOOST_ROOT set to your Boost directory (defaults to /opt/homebrew)



