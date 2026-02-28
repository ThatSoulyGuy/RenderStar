#include <gtest/gtest.h>
#include "RenderStar/Common/Network/Packets/AuthorityChangePacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include "RenderStar/Common/Component/EntityAuthority.hpp"

using namespace RenderStar::Common::Network;
using namespace RenderStar::Common::Network::Packets;

namespace
{
    AuthorityChangePacket RoundTrip(const AuthorityChangePacket& original)
    {
        auto buffer = PacketBuffer::Allocate();
        original.Write(buffer);
        buffer.Reset();
        AuthorityChangePacket result;
        result.Read(buffer);
        return result;
    }
}

TEST(AuthorityChangePacketTest, DefaultValues)
{
    AuthorityChangePacket packet;
    EXPECT_EQ(packet.entityId, -1);
    EXPECT_EQ(packet.authorityLevel, 0);
    EXPECT_EQ(packet.ownerId, -1);
}

TEST(AuthorityChangePacketTest, RoundTripNobody)
{
    AuthorityChangePacket original;
    original.entityId = 10;
    original.authorityLevel = static_cast<uint8_t>(RenderStar::Common::Component::AuthorityLevel::NOBODY);
    original.ownerId = -1;

    auto result = RoundTrip(original);
    EXPECT_EQ(result.entityId, 10);
    EXPECT_EQ(result.authorityLevel, 0);
    EXPECT_EQ(result.ownerId, -1);
}

TEST(AuthorityChangePacketTest, RoundTripServer)
{
    AuthorityChangePacket original;
    original.entityId = 20;
    original.authorityLevel = static_cast<uint8_t>(RenderStar::Common::Component::AuthorityLevel::SERVER);
    original.ownerId = -1;

    auto result = RoundTrip(original);
    EXPECT_EQ(result.entityId, 20);
    EXPECT_EQ(result.authorityLevel, 1);
    EXPECT_EQ(result.ownerId, -1);
}

TEST(AuthorityChangePacketTest, RoundTripClient)
{
    AuthorityChangePacket original;
    original.entityId = 30;
    original.authorityLevel = static_cast<uint8_t>(RenderStar::Common::Component::AuthorityLevel::CLIENT);
    original.ownerId = 5;

    auto result = RoundTrip(original);
    EXPECT_EQ(result.entityId, 30);
    EXPECT_EQ(result.authorityLevel, 2);
    EXPECT_EQ(result.ownerId, 5);
}

TEST(AuthorityChangePacketTest, RoundTripZeroEntityId)
{
    AuthorityChangePacket original;
    original.entityId = 0;
    original.authorityLevel = 1;
    original.ownerId = 0;

    auto result = RoundTrip(original);
    EXPECT_EQ(result.entityId, 0);
    EXPECT_EQ(result.authorityLevel, 1);
    EXPECT_EQ(result.ownerId, 0);
}

TEST(AuthorityChangePacketTest, RoundTripLargeEntityId)
{
    AuthorityChangePacket original;
    original.entityId = 999999;
    original.authorityLevel = 2;
    original.ownerId = 12345;

    auto result = RoundTrip(original);
    EXPECT_EQ(result.entityId, 999999);
    EXPECT_EQ(result.authorityLevel, 2);
    EXPECT_EQ(result.ownerId, 12345);
}

TEST(AuthorityChangePacketTest, AuthorityLevelMatchesEnum)
{
    EXPECT_EQ(static_cast<uint8_t>(RenderStar::Common::Component::AuthorityLevel::NOBODY), 0);
    EXPECT_EQ(static_cast<uint8_t>(RenderStar::Common::Component::AuthorityLevel::SERVER), 1);
    EXPECT_EQ(static_cast<uint8_t>(RenderStar::Common::Component::AuthorityLevel::CLIENT), 2);
}
