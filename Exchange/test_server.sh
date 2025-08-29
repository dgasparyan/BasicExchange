#!/bin/bash

# Test script for UDP Exchange Server
# Usage: ./test_server.sh [port] [host]

PORT=${1:-8080}
HOST=${2:-127.0.0.1}

echo "Testing UDP Exchange Server on $HOST:$PORT"
echo "=========================================="

# Test NewOrder commands (D, UserID, ClientOrderId, Symbol, Quantity, Side, Type, [Price])
echo "Testing NewOrder commands..."
echo "D,user123,1001,AAPL,100,BUY,MARKET" | nc -u -w0 $HOST $PORT
echo "D,user456,1002,MSFT,50,SELL,LIMIT,150.75" | nc -u -w0 $HOST $PORT
echo "D,user789,1003,GOOGL,200,BUY,MARKET" | nc -u -w0 $HOST $PORT

# Test CancelOrder commands (F, UserID, OrigOrderId, Symbol)
echo "Testing CancelOrder commands..."
echo "F,user123,1001,AAPL" | nc -u -w0 $HOST $PORT
echo "F,user456,1002,MSFT" | nc -u -w0 $HOST $PORT

# Test TopOfBook commands (V, UserID, Symbol)
echo "Testing TopOfBook commands..."
echo "V,user123,AAPL" | nc -u -w0 $HOST $PORT
echo "V,user456,MSFT" | nc -u -w0 $HOST $PORT

# Test Quit command (Q)
# echo "Testing Quit command..."
# echo "Q" | nc -u -w0 $HOST $PORT
# sleep 0.5

# Test case variations
echo "Testing case variations..."
echo "d,user101,1004,TSLA,75,sell,limit,250.50" | nc -u -w0 $HOST $PORT
echo "f,user202,1005,NFLX" | nc -u -w0 $HOST $PORT
echo "v,user303,GOOGL" | nc -u -w0 $HOST $PORT

# Test whitespace handling
echo "Testing whitespace handling..."
echo " D , user404 , 1006 , AMZN , 100 , BUY , MARKET " | nc -u -w0 $HOST $PORT
echo " F , user505 , 1007 , META " | nc -u -w0 $HOST $PORT
echo " V , user606 , NVDA " | nc -u -w0 $HOST $PORT



# Test invalid commands
echo "Testing invalid commands..."
echo "X" | nc -u -w0 $HOST $PORT
echo "INVALID" | nc -u -w0 $HOST $PORT

echo "Test completed!" 