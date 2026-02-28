#include <gtest/gtest.h>
#include "RenderStar/Common/Network/Packets/ComponentUpdatePacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"

using namespace RenderStar::Common::Network;
using namespace RenderStar::Common::Network::Packets;

namespace
{
    ComponentUpdatePacket RoundTrip(const ComponentUpdatePacket& original)
    {
        auto buffer = PacketBuffer::Allocate();
        original.Write(buffer);
        buffer.Reset();
        ComponentUpdatePacket result;
        result.Read(buffer);
        return result;
    }
}

TEST(ComponentUpdatePacketTest, DefaultValues)
{
    ComponentUpdatePacket packet;
    EXPECT_EQ(packet.entityId, -1);
    EXPECT_TRUE(packet.xmlData.empty());
}

TEST(ComponentUpdatePacketTest, RoundTripBasic)
{
    ComponentUpdatePacket original;
    original.entityId = 42;
    original.xmlData = "<Entities><Entity id=\"42\"><Transform px=\"1\" py=\"2\" pz=\"3\"/></Entity></Entities>";

    auto result = RoundTrip(original);
    EXPECT_EQ(result.entityId, 42);
    EXPECT_EQ(result.xmlData, original.xmlData);
}

TEST(ComponentUpdatePacketTest, RoundTripEmptyXml)
{
    ComponentUpdatePacket original;
    original.entityId = 0;
    original.xmlData = "";

    auto result = RoundTrip(original);
    EXPECT_EQ(result.entityId, 0);
    EXPECT_TRUE(result.xmlData.empty());
}

TEST(ComponentUpdatePacketTest, RoundTripNegativeEntityId)
{
    ComponentUpdatePacket original;
    original.entityId = -1;
    original.xmlData = "data";

    auto result = RoundTrip(original);
    EXPECT_EQ(result.entityId, -1);
    EXPECT_EQ(result.xmlData, "data");
}

TEST(ComponentUpdatePacketTest, RoundTripLargeXmlData)
{
    ComponentUpdatePacket original;
    original.entityId = 100;
    original.xmlData = std::string(4096, 'X');

    auto result = RoundTrip(original);
    EXPECT_EQ(result.entityId, 100);
    EXPECT_EQ(result.xmlData.size(), 4096u);
    EXPECT_EQ(result.xmlData, original.xmlData);
}

TEST(ComponentUpdatePacketTest, RoundTripSpecialCharactersInXml)
{
    ComponentUpdatePacket original;
    original.entityId = 5;
    original.xmlData = "<Entity name=\"foo&amp;bar\" value=\"1.5\"/>";

    auto result = RoundTrip(original);
    EXPECT_EQ(result.entityId, 5);
    EXPECT_EQ(result.xmlData, original.xmlData);
}

TEST(ComponentUpdatePacketTest, RoundTripMultipleComponents)
{
    ComponentUpdatePacket original;
    original.entityId = 10;
    original.xmlData = "<Entities count=\"1\"><Entity id=\"10\" name=\"Player_0\" authority=\"client\" ownerId=\"0\">"
                        "<Transform px=\"1.5\" py=\"2.5\" pz=\"3.5\" rx=\"0\" ry=\"0\" rz=\"0\" rw=\"1\" sx=\"1\" sy=\"1\" sz=\"1\"/>"
                        "<PlayerIdentity playerId=\"0\"/>"
                        "</Entity></Entities>";

    auto result = RoundTrip(original);
    EXPECT_EQ(result.entityId, 10);
    EXPECT_EQ(result.xmlData, original.xmlData);
}
