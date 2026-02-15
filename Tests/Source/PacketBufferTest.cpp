#include <gtest/gtest.h>
#include "RenderStar/Common/Network/PacketBuffer.hpp"

using namespace RenderStar::Common::Network;

TEST(PacketBufferTest, WriteReadByte)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteByte(std::byte{0xAB});
    EXPECT_EQ(buf.ReadByte(), std::byte{0xAB});
}

TEST(PacketBufferTest, WriteReadBoolean)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteBoolean(true);
    buf.WriteBoolean(false);
    EXPECT_TRUE(buf.ReadBoolean());
    EXPECT_FALSE(buf.ReadBoolean());
}

TEST(PacketBufferTest, WriteReadInt16)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteInt16(12345);
    buf.WriteInt16(-12345);
    EXPECT_EQ(buf.ReadInt16(), 12345);
    EXPECT_EQ(buf.ReadInt16(), -12345);
}

TEST(PacketBufferTest, WriteReadInt32)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteInt32(123456789);
    buf.WriteInt32(-123456789);
    EXPECT_EQ(buf.ReadInt32(), 123456789);
    EXPECT_EQ(buf.ReadInt32(), -123456789);
}

TEST(PacketBufferTest, WriteReadInt64)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteInt64(1234567890123LL);
    EXPECT_EQ(buf.ReadInt64(), 1234567890123LL);
}

TEST(PacketBufferTest, WriteReadFloat)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteFloat(3.14f);
    EXPECT_FLOAT_EQ(buf.ReadFloat(), 3.14f);
}

TEST(PacketBufferTest, WriteReadDouble)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteDouble(2.718281828);
    EXPECT_DOUBLE_EQ(buf.ReadDouble(), 2.718281828);
}

TEST(PacketBufferTest, WriteReadString)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteString("hello world");
    EXPECT_EQ(buf.ReadString(), "hello world");
}

TEST(PacketBufferTest, WriteReadEmptyString)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteString("");
    EXPECT_EQ(buf.ReadString(), "");
}

TEST(PacketBufferTest, SequentialMultipleValues)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteInt32(42);
    buf.WriteFloat(3.14f);
    buf.WriteString("test");
    buf.WriteBoolean(true);

    EXPECT_EQ(buf.ReadInt32(), 42);
    EXPECT_FLOAT_EQ(buf.ReadFloat(), 3.14f);
    EXPECT_EQ(buf.ReadString(), "test");
    EXPECT_TRUE(buf.ReadBoolean());
}

TEST(PacketBufferTest, ReadableBytes)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteInt32(42);
    EXPECT_GT(buf.ReadableBytes(), 0u);
}

TEST(PacketBufferTest, Reset)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteInt32(42);
    buf.ReadInt32();
    EXPECT_EQ(buf.ReadableBytes(), 0u);
    buf.Reset();
    EXPECT_GT(buf.ReadableBytes(), 0u);
    EXPECT_EQ(buf.ReadInt32(), 42);
}

TEST(PacketBufferTest, WriteReadVarint)
{
    auto buf = PacketBuffer::Allocate();
    buf.WriteVarint(300);
    buf.WriteVarint(-1);
    buf.WriteVarint(0);
    EXPECT_EQ(buf.ReadVarint(), 300);
    EXPECT_EQ(buf.ReadVarint(), -1);
    EXPECT_EQ(buf.ReadVarint(), 0);
}

TEST(PacketBufferTest, WrapFromSpan)
{
    auto original = PacketBuffer::Allocate();
    original.WriteInt32(99);
    original.WriteString("wrapped");

    auto span = original.ToSpan();
    auto wrapped = PacketBuffer::Wrap(span);
    EXPECT_EQ(wrapped.ReadInt32(), 99);
    EXPECT_EQ(wrapped.ReadString(), "wrapped");
}
