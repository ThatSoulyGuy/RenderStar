#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include <cstring>
#include <stdexcept>

namespace RenderStar::Common::Network
{
    PacketBuffer PacketBuffer::Allocate(const size_t capacity)
    {
        PacketBuffer packetBuffer;

        packetBuffer.buffer.resize(capacity);
        packetBuffer.readPosition = 0;
        packetBuffer.writePosition = 0;

        return packetBuffer;
    }

    PacketBuffer PacketBuffer::Wrap(std::span<const std::byte> data)
    {
        PacketBuffer packetBuffer;

        packetBuffer.buffer.assign(data.begin(), data.end());
        packetBuffer.readPosition = 0;
        packetBuffer.writePosition = data.size();

        return packetBuffer;
    }

    PacketBuffer& PacketBuffer::WriteByte(const std::byte value)
    {
        EnsureCapacity(1);
        buffer[writePosition++] = value;

        return *this;
    }

    PacketBuffer& PacketBuffer::WriteBoolean(const bool value)
    {
        return WriteByte(value ? std::byte{1} : std::byte{0});
    }

    PacketBuffer& PacketBuffer::WriteInt16(const int16_t value)
    {
        EnsureCapacity(2);

        buffer[writePosition++] = static_cast<std::byte>((value >> 8) & 0xFF);
        buffer[writePosition++] = static_cast<std::byte>(value & 0xFF);

        return *this;
    }

    PacketBuffer& PacketBuffer::WriteInt32(const int32_t value)
    {
        EnsureCapacity(4);

        buffer[writePosition++] = static_cast<std::byte>((value >> 24) & 0xFF);
        buffer[writePosition++] = static_cast<std::byte>((value >> 16) & 0xFF);
        buffer[writePosition++] = static_cast<std::byte>((value >> 8) & 0xFF);
        buffer[writePosition++] = static_cast<std::byte>(value & 0xFF);

        return *this;
    }

    PacketBuffer& PacketBuffer::WriteInt64(const int64_t value)
    {
        EnsureCapacity(8);

        buffer[writePosition++] = static_cast<std::byte>((value >> 56) & 0xFF);
        buffer[writePosition++] = static_cast<std::byte>((value >> 48) & 0xFF);
        buffer[writePosition++] = static_cast<std::byte>((value >> 40) & 0xFF);
        buffer[writePosition++] = static_cast<std::byte>((value >> 32) & 0xFF);
        buffer[writePosition++] = static_cast<std::byte>((value >> 24) & 0xFF);
        buffer[writePosition++] = static_cast<std::byte>((value >> 16) & 0xFF);
        buffer[writePosition++] = static_cast<std::byte>((value >> 8) & 0xFF);
        buffer[writePosition++] = static_cast<std::byte>(value & 0xFF);

        return *this;
    }

    PacketBuffer& PacketBuffer::WriteFloat(const float value)
    {
        int32_t intValue;
        std::memcpy(&intValue, &value, sizeof(float));

        return WriteInt32(intValue);
    }

    PacketBuffer& PacketBuffer::WriteDouble(const double value)
    {
        int64_t intValue;
        std::memcpy(&intValue, &value, sizeof(double));

        return WriteInt64(intValue);
    }

    PacketBuffer& PacketBuffer::WriteVarint(const int32_t value)
    {
        auto unsignedValue = static_cast<uint32_t>(value);

        while (unsignedValue >= 0x80)
        {
            WriteByte(static_cast<std::byte>((unsignedValue & 0x7F) | 0x80));
            unsignedValue >>= 7;
        }

        WriteByte(static_cast<std::byte>(unsignedValue));
        return *this;
    }

    PacketBuffer& PacketBuffer::WriteString(const std::string& value)
    {
        if (value.length() > MAX_STRING_LENGTH)
            throw std::runtime_error("String exceeds maximum length");

        WriteVarint(static_cast<int32_t>(value.length()));

        EnsureCapacity(value.length());

        for (char character : value)
            buffer[writePosition++] = static_cast<std::byte>(character);

        return *this;
    }

    PacketBuffer& PacketBuffer::WriteBytes(const std::span<const std::byte> data)
    {
        EnsureCapacity(data.size());

        for (std::byte byte : data)
            buffer[writePosition++] = byte;

        return *this;
    }

    std::byte PacketBuffer::ReadByte()
    {
        if (readPosition >= writePosition)
            throw std::runtime_error("Buffer underflow");

        return buffer[readPosition++];
    }

    bool PacketBuffer::ReadBoolean()
    {
        return ReadByte() != std::byte{0};
    }

    int16_t PacketBuffer::ReadInt16()
    {
        int16_t value = 0;

        value |= static_cast<int16_t>(static_cast<uint8_t>(ReadByte())) << 8;
        value |= static_cast<int16_t>(static_cast<uint8_t>(ReadByte()));

        return value;
    }

    int32_t PacketBuffer::ReadInt32()
    {
        int32_t value = 0;

        value |= static_cast<int32_t>(static_cast<uint8_t>(ReadByte())) << 24;
        value |= static_cast<int32_t>(static_cast<uint8_t>(ReadByte())) << 16;
        value |= static_cast<int32_t>(static_cast<uint8_t>(ReadByte())) << 8;
        value |= static_cast<int32_t>(static_cast<uint8_t>(ReadByte()));

        return value;
    }

    int64_t PacketBuffer::ReadInt64()
    {
        int64_t value = 0;

        value |= static_cast<int64_t>(static_cast<uint8_t>(ReadByte())) << 56;
        value |= static_cast<int64_t>(static_cast<uint8_t>(ReadByte())) << 48;
        value |= static_cast<int64_t>(static_cast<uint8_t>(ReadByte())) << 40;
        value |= static_cast<int64_t>(static_cast<uint8_t>(ReadByte())) << 32;
        value |= static_cast<int64_t>(static_cast<uint8_t>(ReadByte())) << 24;
        value |= static_cast<int64_t>(static_cast<uint8_t>(ReadByte())) << 16;
        value |= static_cast<int64_t>(static_cast<uint8_t>(ReadByte())) << 8;
        value |= static_cast<int64_t>(static_cast<uint8_t>(ReadByte()));

        return value;
    }

    float PacketBuffer::ReadFloat()
    {
        const int32_t intValue = ReadInt32();

        float floatValue;

        std::memcpy(&floatValue, &intValue, sizeof(float));

        return floatValue;
    }

    double PacketBuffer::ReadDouble()
    {
        const int64_t intValue = ReadInt64();

        double doubleValue;

        std::memcpy(&doubleValue, &intValue, sizeof(double));

        return doubleValue;
    }

    int32_t PacketBuffer::ReadVarint()
    {
        uint32_t result = 0;
        int32_t shift = 0;

        while (true)
        {
            const auto byte = static_cast<uint8_t>(ReadByte());

            result |= static_cast<uint32_t>(byte & 0x7F) << shift;

            if ((byte & 0x80) == 0)
                break;

            shift += 7;

            if (shift >= 32)
                throw std::runtime_error("Varint too long");
        }

        return static_cast<int32_t>(result);
    }

    std::string PacketBuffer::ReadString()
    {
        const int32_t length = ReadVarint();

        if (length < 0 || length > static_cast<int32_t>(MAX_STRING_LENGTH))
            throw std::runtime_error("Invalid string length");

        std::string result;
        result.reserve(length);

        for (int32_t index = 0; index < length; ++index)
            result.push_back(static_cast<char>(ReadByte()));

        return result;
    }

    std::vector<std::byte> PacketBuffer::ReadBytes(const size_t length)
    {
        std::vector<std::byte> result;
        result.reserve(length);

        for (size_t index = 0; index < length; ++index)
            result.push_back(ReadByte());

        return result;
    }

    std::span<const std::byte> PacketBuffer::ToSpan() const
    {
        return { buffer.data(), writePosition };
    }

    size_t PacketBuffer::ReadableBytes() const
    {
        return writePosition - readPosition;
    }

    size_t PacketBuffer::WritableBytes() const
    {
        return buffer.size() - writePosition;
    }

    void PacketBuffer::Reset()
    {
        readPosition = 0;
    }

    void PacketBuffer::EnsureCapacity(size_t additionalBytes)
    {
        size_t requiredCapacity = writePosition + additionalBytes;

        if (requiredCapacity > buffer.size())
        {
            size_t newCapacity = buffer.size() * 2;

            while (newCapacity < requiredCapacity)
                newCapacity *= 2;

            buffer.resize(newCapacity);
        }
    }
}
