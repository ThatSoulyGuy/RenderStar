#include <gtest/gtest.h>
#include "RenderStar/Client/Network/ConnectionState.hpp"

using namespace RenderStar::Client::Network;

TEST(ConnectionStateTest, AllValuesExist)
{
    ConnectionState a = ConnectionState::DISCONNECTED;
    ConnectionState b = ConnectionState::CONNECTING;
    ConnectionState c = ConnectionState::CONNECTED;
    ConnectionState d = ConnectionState::DISCONNECTING;

    EXPECT_NE(a, b);
    EXPECT_NE(b, c);
    EXPECT_NE(c, d);
    EXPECT_NE(a, d);
}

TEST(ConnectionStateTest, CastToUint8)
{
    EXPECT_EQ(static_cast<uint8_t>(ConnectionState::DISCONNECTED), 0);
    EXPECT_EQ(static_cast<uint8_t>(ConnectionState::CONNECTING), 1);
    EXPECT_EQ(static_cast<uint8_t>(ConnectionState::CONNECTED), 2);
    EXPECT_EQ(static_cast<uint8_t>(ConnectionState::DISCONNECTING), 3);
}

TEST(ConnectionStateTest, AssignAndCompare)
{
    ConnectionState state = ConnectionState::CONNECTING;
    EXPECT_EQ(state, ConnectionState::CONNECTING);
    state = ConnectionState::CONNECTED;
    EXPECT_EQ(state, ConnectionState::CONNECTED);
}
