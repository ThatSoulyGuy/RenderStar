#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace RenderStar::Common::Network
{
    class PacketBuffer
    {
    public:

        static constexpr size_t DEFAULT_CAPACITY = 256;
        static constexpr size_t MAX_STRING_LENGTH = 32767;

        static PacketBuffer Allocate(size_t capacity = DEFAULT_CAPACITY);

        static PacketBuffer Wrap(std::span<const std::byte> data);

        PacketBuffer& WriteByte(std::byte value);

        PacketBuffer& WriteBoolean(bool value);

        PacketBuffer& WriteInt16(int16_t value);

        PacketBuffer& WriteInt32(int32_t value);

        PacketBuffer& WriteInt64(int64_t value);

        PacketBuffer& WriteFloat(float value);

        PacketBuffer& WriteDouble(double value);

        PacketBuffer& WriteVarint(int32_t value);

        PacketBuffer& WriteString(const std::string& value);

        PacketBuffer& WriteBytes(std::span<const std::byte> data);

        std::byte ReadByte();

        bool ReadBoolean();

        int16_t ReadInt16();

        int32_t ReadInt32();

        int64_t ReadInt64();

        float ReadFloat();

        double ReadDouble();

        int32_t ReadVarint();

        std::string ReadString();

        std::vector<std::byte> ReadBytes(size_t length);

        std::span<const std::byte> ToSpan() const;

        size_t ReadableBytes() const;

        size_t WritableBytes() const;

        void Reset();

    private:

        PacketBuffer() = default;

        void EnsureCapacity(size_t additionalBytes);

        std::vector<std::byte> buffer;
        size_t readPosition;
        size_t writePosition;
    };
}
