#!/bin/bash

# Large-scale test script for UDP Exchange Server with 100 orders
# Usage: ./test_server_100_orders.sh [port] [host]

PORT=${1:-8080}
HOST=${2:-127.0.0.1}

echo "Testing UDP Exchange Server on $HOST:$PORT with 100 orders"
echo "========================================================="

# Array of supported stock symbols (matching OrderBookManager)
SYMBOLS=("AAPL" "GOOGL" "MSFT" "AMZN" "META" "NVDA")

# Array of user IDs
USERS=("user001" "user002" "user003" "user004" "user005" "user006" "user007" "user008" "user009" "user010")

# Array of order types
ORDER_TYPES=("MARKET" "LIMIT")

# Array of sides
SIDES=("BUY" "SELL")

# Function to generate random price
generate_price() {
    local base_price=$((RANDOM % 1000 + 50))
    local cents=$((RANDOM % 100))
    echo "$base_price.$cents"
}

# Function to generate random quantity
generate_quantity() {
    local quantity=$((RANDOM % 1000 + 10))
    echo $quantity
}

echo "Generating 100 orders..."

# Generate 100 NewOrder commands
for i in {1..100}; do
    # Select random values
    SYMBOL=${SYMBOLS[$((RANDOM % ${#SYMBOLS[@]}))]}
    USER=${USERS[$((RANDOM % ${#USERS[@]}))]}
    SIDE=${SIDES[$((RANDOM % ${#SIDES[@]}))]}
    ORDER_TYPE=${ORDER_TYPES[$((RANDOM % ${#ORDER_TYPES[@]}))]}
    QUANTITY=$(generate_quantity)
    CLIENT_ORDER_ID=$((1000 + i))
    
    if [ "$ORDER_TYPE" = "LIMIT" ]; then
        PRICE=$(generate_price)
        echo "D,$USER,$CLIENT_ORDER_ID,$SYMBOL,$QUANTITY,$SIDE,$ORDER_TYPE,$PRICE" | nc -u -w0 $HOST $PORT
    else
        echo "D,$USER,$CLIENT_ORDER_ID,$SYMBOL,$QUANTITY,$SIDE,$ORDER_TYPE" | nc -u -w0 $HOST $PORT
    fi
    
    # Small delay to prevent overwhelming the server
    sleep 0.01
done

echo "Generated 100 NewOrder commands"

# Generate some CancelOrder commands for existing orders
echo "Generating CancelOrder commands..."
for i in {1..20}; do
    USER=${USERS[$((RANDOM % ${#USERS[@]}))]}
    CLIENT_ORDER_ID=$((1000 + RANDOM % 100))
    SYMBOL=${SYMBOLS[$((RANDOM % ${#SYMBOLS[@]}))]}
    ORIG_ORDER_ID=$((2000 + RANDOM % 1000))
    
    echo "F,$USER,$CLIENT_ORDER_ID,$SYMBOL,$ORIG_ORDER_ID" | nc -u -w0 $HOST $PORT
    sleep 0.01
done

# Generate some TopOfBook queries
echo "Generating TopOfBook queries..."
for i in {1..30}; do
    USER=${USERS[$((RANDOM % ${#USERS[@]}))]}
    CLIENT_ORDER_ID=$((2000 + i))
    SYMBOL=${SYMBOLS[$((RANDOM % ${#SYMBOLS[@]}))]}
    
    echo "V,$USER,$CLIENT_ORDER_ID,$SYMBOL" | nc -u -w0 $HOST $PORT
    sleep 0.01
done

# Test some edge cases
echo "Testing edge cases..."
echo "D,user999,9999,NVDA,1,BUY,LIMIT,999.99" | nc -u -w0 $HOST $PORT
echo "D,user888,8888,MSFT,999999,SELL,MARKET" | nc -u -w0 $HOST $PORT
echo "D,user777,7777,AAPL,0,BUY,LIMIT,0.01" | nc -u -w0 $HOST $PORT

# Test case variations
echo "Testing case variations..."
echo "d,user101,1001,META,75,sell,limit,250.50" | nc -u -w0 $HOST $PORT
echo "f,user202,1002,AMZN,2005" | nc -u -w0 $HOST $PORT
echo "v,user303,1003,GOOGL" | nc -u -w0 $HOST $PORT

# Test whitespace handling
echo "Testing whitespace handling..."
echo " D , user404 , 1004 , AMZN , 100 , BUY , MARKET " | nc -u -w0 $HOST $PORT
echo " F , user505 , 1005 , META , 2005 " | nc -u -w0 $HOST $PORT
echo " V , user606 , 1006 , NVDA " | nc -u -w0 $HOST $PORT

# Test invalid commands
echo "Testing invalid commands..."
echo "X" | nc -u -w0 $HOST $PORT
echo "INVALID" | nc -u -w0 $HOST $PORT
echo "D,user123" | nc -u -w0 $HOST $PORT
echo "D,user123,1001,AAPL,100,BUY,INVALID" | nc -u -w0 $HOST $PORT

echo "Large-scale test completed! Generated 100+ orders and various commands."
echo "Total commands sent: ~160+" 