# Unit Testing with Google Test

This directory contains unit tests for the Exchange project using Google Test framework.

## Prerequisites

### Installing Google Test

#### On macOS (using Homebrew):
```bash
brew install googletest
```

#### On Ubuntu/Debian:
```bash
sudo apt-get install libgtest-dev
cd /usr/src/googletest
sudo cmake .
sudo make
sudo cp lib/*.a /usr/lib
```

#### On CentOS/RHEL:
```bash
sudo yum install gtest-devel
```

#### Building from source:
```bash
git clone https://github.com/google/googletest.git
cd googletest
mkdir build && cd build
cmake ..
make
sudo make install
```

## Building and Running Tests

### Using Makefile (Recommended)

1. **Build and run all tests:**
   ```bash
   cd test
   make test
   ```

2. **Build only:**
   ```bash
   make all
   ```

3. **Run with verbose output:**
   ```bash
   make test-verbose
   ```

4. **Run specific tests:**
   ```bash
   make test-filter FILTER=CommonUtilsTest*
   ```

5. **Clean build artifacts:**
   ```bash
   make clean
   ```

### Using CMake

1. **Create build directory:**
   ```bash
   cd test
   mkdir build && cd build
   ```

2. **Configure and build:**
   ```bash
   cmake ..
   make
   ```

3. **Run tests:**
   ```bash
   ./run_tests
   ```

## Test Structure

### Test Files

- `test_common_utils.cpp` - Tests for CommonUtils functions
- `test_order_utils.cpp` - Tests for OrderUtils functions  
- `test_events.cpp` - Tests for Event classes
- `test_event_parser.cpp` - Tests for EventParser (to be implemented)

### Test Organization

Each test file follows this structure:

```cpp
#include <gtest/gtest.h>
#include "YourHeader.h"

namespace Exchange {
namespace test {

class YourClassTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code before each test
    }

    void TearDown() override {
        // Cleanup code after each test
    }
};

TEST_F(YourClassTest, TestName) {
    // Test implementation
    EXPECT_EQ(actual, expected);
}

} // namespace test
} // namespace Exchange
```

## Google Test Macros

### Basic Assertions

- `EXPECT_EQ(actual, expected)` - Equal
- `EXPECT_NE(actual, expected)` - Not equal
- `EXPECT_LT(actual, expected)` - Less than
- `EXPECT_LE(actual, expected)` - Less than or equal
- `EXPECT_GT(actual, expected)` - Greater than
- `EXPECT_GE(actual, expected)` - Greater than or equal

### String Assertions

- `EXPECT_STREQ(str1, str2)` - String equal
- `EXPECT_STRNE(str1, str2)` - String not equal
- `EXPECT_STRCASEEQ(str1, str2)` - Case-insensitive string equal

### Boolean Assertions

- `EXPECT_TRUE(condition)` - True
- `EXPECT_FALSE(condition)` - False

### Floating Point Assertions

- `EXPECT_DOUBLE_EQ(actual, expected)` - Double equal
- `EXPECT_NEAR(actual, expected, abs_error)` - Near with tolerance

### Exception Assertions

- `EXPECT_THROW(statement, exception_type)` - Expects exception
- `EXPECT_NO_THROW(statement)` - Expects no exception

## Test Fixtures

Use test fixtures for shared setup:

```cpp
class MyTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup
    }

    void TearDown() override {
        // Common cleanup
    }

    // Shared test data
    std::string testData;
};

TEST_F(MyTestFixture, Test1) {
    // Uses shared setup
}

TEST_F(MyTestFixture, Test2) {
    // Uses shared setup
}
```

## Running Specific Tests

### Command Line Options

```bash
# Run all tests
./run_tests

# Run specific test case
./run_tests --gtest_filter=CommonUtilsTest*

# Run specific test method
./run_tests --gtest_filter=CommonUtilsTest.TrimCopy_EmptyString

# Run tests with verbose output
./run_tests --gtest_verbose

# Run tests and show timing
./run_tests --gtest_print_time=1

# Run tests in random order
./run_tests --gtest_shuffle

# Run tests multiple times
./run_tests --gtest_repeat=3
```

### Test Discovery

- Tests are automatically discovered based on naming conventions
- Test classes should inherit from `::testing::Test`
- Test methods should start with `TEST` or `TEST_F`
- Test names should be descriptive and follow the pattern `ClassName_TestName`

## Best Practices

1. **Test Naming**: Use descriptive names that explain what is being tested
2. **One Assertion Per Test**: Each test should verify one specific behavior
3. **Arrange-Act-Assert**: Structure tests with setup, execution, and verification
4. **Test Independence**: Tests should not depend on each other
5. **Fast Tests**: Keep tests fast to encourage frequent running
6. **Clear Error Messages**: Use descriptive assertion messages

## Example Test

```cpp
TEST_F(OrderUtilsTest, ToSide_ValidBuy) {
    // Arrange
    std::string input = "BUY";
    
    // Act
    Side result = toSide(input);
    
    // Assert
    EXPECT_EQ(result, Side::Buy);
}
```

## Troubleshooting

### Common Issues

1. **Google Test not found**: Ensure Google Test is properly installed and linked
2. **Boost not found**: Check Boost installation and paths
3. **Compilation errors**: Verify all dependencies are included
4. **Test failures**: Check test logic and expected values

### Debugging Tests

```bash
# Run with debug output
./run_tests --gtest_verbose --gtest_break_on_failure

# Run specific failing test
./run_tests --gtest_filter=*FailingTest* --gtest_verbose
``` 