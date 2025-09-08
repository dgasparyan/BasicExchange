cmake_minimum_required(VERSION 3.16)
project(ExchangeTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GTest
find_package(GTest REQUIRED)

# Include directories
include_directories(../include)

# Find Boost
find_package(Boost REQUIRED)

# Test source files
set(TEST_SOURCES
    test_common_utils.cpp
    test_order_utils.cpp
    test_event_parser.cpp
    test_events.cpp
    test_orderbook.cpp
)

# Create test executable
add_executable(run_tests ${TEST_SOURCES})

# Link libraries
target_link_libraries(run_tests 
    GTest::gtest 
    GTest::gtest_main
    GTest::gmock
    Boost::boost
)

# Add source files from main project
target_sources(run_tests PRIVATE
    ../src/CommonUtils.cpp
    ../src/OrderUtils.cpp
    ../src/EventParser.cpp
    ../src/Event.cpp
    ../src/OrderBook.cpp
    ../src/Order.cpp
    ../src/ReportSink.cpp
)

# Enable testing
enable_testing()
add_test(NAME ExchangeTests COMMAND run_tests) 