#include <gtest/gtest.h>
#include "RenderStar/Common/Network/PacketBuffer.hpp"

using namespace RenderStar::Common::Network;

class PacketBufferTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
    }
};

TEST_F(PacketBufferTest, WriteAndReadByte)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteByte(std::byte{42});
    buffer.WriteByte(std::byte{0});
    buffer.WriteByte(std::byte{255});

    buffer.Reset();

    EXPECT_EQ(buffer.ReadByte(), std::byte{42});
    EXPECT_EQ(buffer.ReadByte(), std::byte{0});
    EXPECT_EQ(buffer.ReadByte(), std::byte{255});
}

TEST_F(PacketBufferTest, WriteAndReadInt16)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteInt16(1234);
    buffer.WriteInt16(-32768);
    buffer.WriteInt16(32767);

    buffer.Reset();

    EXPECT_EQ(buffer.ReadInt16(), 1234);
    EXPECT_EQ(buffer.ReadInt16(), -32768);
    EXPECT_EQ(buffer.ReadInt16(), 32767);
}

TEST_F(PacketBufferTest, WriteAndReadInt32)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteInt32(123456789);
    buffer.WriteInt32(-2147483648);
    buffer.WriteInt32(2147483647);

    buffer.Reset();

    EXPECT_EQ(buffer.ReadInt32(), 123456789);
    EXPECT_EQ(buffer.ReadInt32(), -2147483648);
    EXPECT_EQ(buffer.ReadInt32(), 2147483647);
}

TEST_F(PacketBufferTest, WriteAndReadInt64)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteInt64(1234567890123456789LL);
    buffer.WriteInt64(-9223372036854775807LL);

    buffer.Reset();

    EXPECT_EQ(buffer.ReadInt64(), 1234567890123456789LL);
    EXPECT_EQ(buffer.ReadInt64(), -9223372036854775807LL);
}

TEST_F(PacketBufferTest, WriteAndReadFloat)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteFloat(3.14159f);
    buffer.WriteFloat(-1.0f);
    buffer.WriteFloat(0.0f);

    buffer.Reset();

    EXPECT_FLOAT_EQ(buffer.ReadFloat(), 3.14159f);
    EXPECT_FLOAT_EQ(buffer.ReadFloat(), -1.0f);
    EXPECT_FLOAT_EQ(buffer.ReadFloat(), 0.0f);
}

TEST_F(PacketBufferTest, WriteAndReadDouble)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteDouble(3.141592653589793);
    buffer.WriteDouble(-1.0);

    buffer.Reset();

    EXPECT_DOUBLE_EQ(buffer.ReadDouble(), 3.141592653589793);
    EXPECT_DOUBLE_EQ(buffer.ReadDouble(), -1.0);
}

TEST_F(PacketBufferTest, WriteAndReadString)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteString("Hello, World!");
    buffer.WriteString("");
    buffer.WriteString("Test");

    buffer.Reset();

    EXPECT_EQ(buffer.ReadString(), "Hello, World!");
    EXPECT_EQ(buffer.ReadString(), "");
    EXPECT_EQ(buffer.ReadString(), "Test");
}

TEST_F(PacketBufferTest, WriteAndReadBoolean)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteBoolean(true);
    buffer.WriteBoolean(false);
    buffer.WriteBoolean(true);

    buffer.Reset();

    EXPECT_TRUE(buffer.ReadBoolean());
    EXPECT_FALSE(buffer.ReadBoolean());
    EXPECT_TRUE(buffer.ReadBoolean());
}

TEST_F(PacketBufferTest, WriteAndReadVarint)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteVarint(0);
    buffer.WriteVarint(127);
    buffer.WriteVarint(128);
    buffer.WriteVarint(16383);
    buffer.WriteVarint(16384);
    buffer.WriteVarint(2097151);
    buffer.WriteVarint(-1);

    buffer.Reset();

    EXPECT_EQ(buffer.ReadVarint(), 0);
    EXPECT_EQ(buffer.ReadVarint(), 127);
    EXPECT_EQ(buffer.ReadVarint(), 128);
    EXPECT_EQ(buffer.ReadVarint(), 16383);
    EXPECT_EQ(buffer.ReadVarint(), 16384);
    EXPECT_EQ(buffer.ReadVarint(), 2097151);
    EXPECT_EQ(buffer.ReadVarint(), -1);
}

TEST_F(PacketBufferTest, MixedTypesRoundTrip)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteInt32(42);
    buffer.WriteString("Hello");
    buffer.WriteFloat(3.14f);
    buffer.WriteBoolean(true);
    buffer.WriteInt64(9876543210LL);

    buffer.Reset();

    EXPECT_EQ(buffer.ReadInt32(), 42);
    EXPECT_EQ(buffer.ReadString(), "Hello");
    EXPECT_FLOAT_EQ(buffer.ReadFloat(), 3.14f);
    EXPECT_TRUE(buffer.ReadBoolean());
    EXPECT_EQ(buffer.ReadInt64(), 9876543210LL);
}

TEST_F(PacketBufferTest, ToSpanReturnsCorrectSize)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteInt32(42);
    buffer.WriteInt32(84);

    auto span = buffer.ToSpan();
    EXPECT_EQ(span.size(), 8);
}

TEST_F(PacketBufferTest, WrapExistingData)
{
    PacketBuffer original = PacketBuffer::Allocate();
    original.WriteInt32(42);
    original.WriteString("Test");

    auto span = original.ToSpan();
    PacketBuffer restored = PacketBuffer::Wrap(span);

    EXPECT_EQ(restored.ReadInt32(), 42);
    EXPECT_EQ(restored.ReadString(), "Test");
}
