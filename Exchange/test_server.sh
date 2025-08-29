#!/bin/bash

# Test script for UDP Exchange Server
# Usage: ./test_server.sh [port] [host]

PORT=${1:-8080}
HOST=${2:-127.0.0.1}

echo "Testing UDP Exchange Server on $HOST:$PORT"
echo "=========================================="

# Test valid commands
echo "Testing valid commands..."
echo "D" | nc -u -w0 $HOST $PORT
echo "F" | nc -u -w0 $HOST $PORT
echo "V" | nc -u -w0 $HOST $PORT

# Test case variations
echo "Testing case variations..."
echo "d" | nc -u -w0 $HOST $PORT
echo "f" | nc -u -w0 $HOST $PORT
echo "v" | nc -u -w0 $HOST $PORT

# Test whitespace handling
echo "Testing whitespace handling..."
echo "  D  " | nc -u -w0 $HOST $PORT
echo "  F  " | nc -u -w0 $HOST $PORT



# Test invalid commands
echo "Testing invalid commands..."
echo "X" | nc -u -w0 $HOST $PORT
sleep 1
echo "INVALID" | nc -u -w0 $HOST $PORT
sleep 1

echo "Test completed!" 