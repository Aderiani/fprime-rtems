// ======================================================================
// \title Os/Posix/test/ut/PosixFileTests.cpp
// \brief tests using posix implementation for Os::File interface testing
// ======================================================================
#include <gtest/gtest.h>
#include "Fw/Types/String.hpp"
#include "Os/Os.hpp"
#include "Os/Posix/Queue.hpp"
#include "Os/Queue.hpp"
#include "STest/Random/Random.hpp"

// Construction test
TEST(Interface, Construction) {}

// Destruct test
TEST(Interface, Destruction) {}

// Create test
TEST(Interface, Create) {}

// Send test
TEST(Interface, SendPointer) {}

// Send test
TEST(Interface, SendBuffer) {}

// Receive test
TEST(Interface, ReceivePointer) {}

// Receive test
TEST(Interface, ReceiveBuffer) {}

TEST(Interface, MessageCount) {}

TEST(Interface, MessageHighWaterMarkCount) {}

TEST(Interface, QueueHandle) {}

int main(int argc, char** argv) {
    Os::init();
    ::testing::InitGoogleTest(&argc, argv);
    STest::Random::seed();
    return RUN_ALL_TESTS();
}
