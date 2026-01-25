#include <gtest/gtest.h>
#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include "RenderStar/Common/Network/PacketModule.hpp"
#include "RenderStar/Common/Network/IPacket.hpp"

#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <atomic>
#include <numeric>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace RenderStar::Common::Network;
using namespace std::chrono;
using namespace std::chrono_literals;

// =============================================================================
// Test Packet Definitions
// =============================================================================

class EmptyPacket : public IPacket
{
public:
    void Write(PacketBuffer& buffer) const override { }
    void Read(PacketBuffer& buffer) override { }
};

class SimplePacket : public IPacket
{
public:
    int32_t value = 0;
    std::string message;

    SimplePacket() = default;
    SimplePacket(int32_t v, std::string msg) : value(v), message(std::move(msg)) { }

    void Write(PacketBuffer& buffer) const override
    {
        buffer.WriteInt32(value);
        buffer.WriteString(message);
    }

    void Read(PacketBuffer& buffer) override
    {
        value = buffer.ReadInt32();
        message = buffer.ReadString();
    }
};

class PositionPacket : public IPacket
{
public:
    double x = 0.0, y = 0.0, z = 0.0;
    float yaw = 0.0f, pitch = 0.0f;
    bool onGround = false;

    PositionPacket() = default;
    PositionPacket(double x, double y, double z, float yaw, float pitch, bool ground)
        : x(x), y(y), z(z), yaw(yaw), pitch(pitch), onGround(ground) { }

    void Write(PacketBuffer& buffer) const override
    {
        buffer.WriteDouble(x);
        buffer.WriteDouble(y);
        buffer.WriteDouble(z);
        buffer.WriteFloat(yaw);
        buffer.WriteFloat(pitch);
        buffer.WriteBoolean(onGround);
    }

    void Read(PacketBuffer& buffer) override
    {
        x = buffer.ReadDouble();
        y = buffer.ReadDouble();
        z = buffer.ReadDouble();
        yaw = buffer.ReadFloat();
        pitch = buffer.ReadFloat();
        onGround = buffer.ReadBoolean();
    }
};

class ChunkDataPacket : public IPacket
{
public:
    int32_t chunkX = 0, chunkZ = 0;
    std::vector<uint8_t> blockData;

    ChunkDataPacket() = default;
    ChunkDataPacket(int32_t x, int32_t z, size_t dataSize)
        : chunkX(x), chunkZ(z), blockData(dataSize, 0) { }

    void Write(PacketBuffer& buffer) const override
    {
        buffer.WriteInt32(chunkX);
        buffer.WriteInt32(chunkZ);
        buffer.WriteVarint(static_cast<int32_t>(blockData.size()));

        for (uint8_t byte : blockData)
            buffer.WriteByte(std::byte{byte});
    }

    void Read(PacketBuffer& buffer) override
    {
        chunkX = buffer.ReadInt32();
        chunkZ = buffer.ReadInt32();
        int32_t size = buffer.ReadVarint();
        blockData.resize(size);

        for (int32_t i = 0; i < size; ++i)
            blockData[i] = static_cast<uint8_t>(buffer.ReadByte());
    }
};

class EntityUpdatePacket : public IPacket
{
public:
    int32_t entityId = 0;
    int16_t deltaX = 0, deltaY = 0, deltaZ = 0;
    int8_t velocityX = 0, velocityY = 0, velocityZ = 0;
    bool hasVelocity = false;

    void Write(PacketBuffer& buffer) const override
    {
        buffer.WriteVarint(entityId);
        buffer.WriteInt16(deltaX);
        buffer.WriteInt16(deltaY);
        buffer.WriteInt16(deltaZ);
        buffer.WriteBoolean(hasVelocity);

        if (hasVelocity)
        {
            buffer.WriteByte(std::byte(velocityX));
            buffer.WriteByte(std::byte(velocityY));
            buffer.WriteByte(std::byte(velocityZ));
        }
    }

    void Read(PacketBuffer& buffer) override
    {
        entityId = buffer.ReadVarint();
        deltaX = buffer.ReadInt16();
        deltaY = buffer.ReadInt16();
        deltaZ = buffer.ReadInt16();
        hasVelocity = buffer.ReadBoolean();

        if (hasVelocity)
        {
            velocityX = static_cast<int8_t>(buffer.ReadByte());
            velocityY = static_cast<int8_t>(buffer.ReadByte());
            velocityZ = static_cast<int8_t>(buffer.ReadByte());
        }
    }
};

// =============================================================================
// Test Fixtures
// =============================================================================

class NetworkingTest : public ::testing::Test
{
protected:
    PacketModule packetModule;
    std::mt19937 rng{42};

    void SetUp() override
    {
        packetModule.RegisterPacket<EmptyPacket>(0x00);
        packetModule.RegisterPacket<SimplePacket>(0x01);
        packetModule.RegisterPacket<PositionPacket>(0x02);
        packetModule.RegisterPacket<ChunkDataPacket>(0x03);
        packetModule.RegisterPacket<EntityUpdatePacket>(0x04);
    }

    template<typename T>
    double MeasureSerializationTimeMs(const T& packet, int iterations = 1000)
    {
        auto start = high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i)
        {
            PacketBuffer buffer = packetModule.Serialize(packet);
            (void)buffer;
        }

        auto end = high_resolution_clock::now();
        return duration<double, std::milli>(end - start).count() / iterations;
    }

    template<typename T>
    double MeasureDeserializationTimeMs(PacketBuffer& serialized, int iterations = 1000)
    {
        auto start = high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i)
        {
            serialized.Reset();
            auto packet = packetModule.Deserialize(serialized);
            (void)packet;
        }

        auto end = high_resolution_clock::now();
        return duration<double, std::milli>(end - start).count() / iterations;
    }

    double MeasureRoundTripTimeMs(const IPacket& packet, int iterations = 1000)
    {
        auto start = high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i)
        {
            PacketBuffer buffer = packetModule.Serialize(packet);
            buffer.Reset();
            auto deserialized = packetModule.Deserialize(buffer);
            (void)deserialized;
        }

        auto end = high_resolution_clock::now();
        return duration<double, std::milli>(end - start).count() / iterations;
    }
};

// =============================================================================
// Basic Packet Module Tests
// =============================================================================

TEST_F(NetworkingTest, RegisterAndCreatePacket)
{
    auto packet = packetModule.CreatePacket(0x01);
    ASSERT_NE(packet, nullptr);

    auto* simplePacket = dynamic_cast<SimplePacket*>(packet.get());
    ASSERT_NE(simplePacket, nullptr);
}

TEST_F(NetworkingTest, CreateUnregisteredPacketReturnsNull)
{
    auto packet = packetModule.CreatePacket(0xFF);
    EXPECT_EQ(packet, nullptr);
}

TEST_F(NetworkingTest, SerializeDeserializeEmptyPacket)
{
    EmptyPacket original;
    PacketBuffer buffer = packetModule.Serialize(original);
    buffer.Reset();

    auto restored = packetModule.Deserialize(buffer);
    ASSERT_NE(restored, nullptr);
    EXPECT_NE(dynamic_cast<EmptyPacket*>(restored.get()), nullptr);
}

TEST_F(NetworkingTest, SerializeDeserializeSimplePacket)
{
    SimplePacket original(42, "Hello, Network!");
    PacketBuffer buffer = packetModule.Serialize(original);
    buffer.Reset();

    auto restored = packetModule.Deserialize(buffer);
    ASSERT_NE(restored, nullptr);

    auto* simplePacket = dynamic_cast<SimplePacket*>(restored.get());
    ASSERT_NE(simplePacket, nullptr);
    EXPECT_EQ(simplePacket->value, 42);
    EXPECT_EQ(simplePacket->message, "Hello, Network!");
}

TEST_F(NetworkingTest, SerializeDeserializePositionPacket)
{
    PositionPacket original(1.5, 64.0, -3.25, 90.0f, -45.0f, true);
    PacketBuffer buffer = packetModule.Serialize(original);
    buffer.Reset();

    auto restored = packetModule.Deserialize(buffer);
    ASSERT_NE(restored, nullptr);

    auto* posPacket = dynamic_cast<PositionPacket*>(restored.get());
    ASSERT_NE(posPacket, nullptr);
    EXPECT_DOUBLE_EQ(posPacket->x, 1.5);
    EXPECT_DOUBLE_EQ(posPacket->y, 64.0);
    EXPECT_DOUBLE_EQ(posPacket->z, -3.25);
    EXPECT_FLOAT_EQ(posPacket->yaw, 90.0f);
    EXPECT_FLOAT_EQ(posPacket->pitch, -45.0f);
    EXPECT_TRUE(posPacket->onGround);
}

TEST_F(NetworkingTest, SerializeDeserializeLargePacket)
{
    ChunkDataPacket original(10, -5, 16384);

    std::uniform_int_distribution<int> dist(0, 255);
    for (auto& byte : original.blockData)
        byte = static_cast<uint8_t>(dist(rng));

    PacketBuffer buffer = packetModule.Serialize(original);
    buffer.Reset();

    auto restored = packetModule.Deserialize(buffer);
    ASSERT_NE(restored, nullptr);

    auto* chunkPacket = dynamic_cast<ChunkDataPacket*>(restored.get());
    ASSERT_NE(chunkPacket, nullptr);
    EXPECT_EQ(chunkPacket->chunkX, 10);
    EXPECT_EQ(chunkPacket->chunkZ, -5);
    EXPECT_EQ(chunkPacket->blockData.size(), 16384u);
    EXPECT_EQ(chunkPacket->blockData, original.blockData);
}

TEST_F(NetworkingTest, PacketHandlerInvocation)
{
    bool handlerCalled = false;
    int32_t receivedValue = 0;

    packetModule.RegisterHandler<SimplePacket>([&](SimplePacket& packet)
    {
        handlerCalled = true;
        receivedValue = packet.value;
    });

    SimplePacket packet(123, "Test");
    packetModule.HandlePacket(packet);

    EXPECT_TRUE(handlerCalled);
    EXPECT_EQ(receivedValue, 123);
}

// =============================================================================
// TPS Simulation Tests
// =============================================================================

struct TPSMetrics
{
    double averageTickTimeMs;
    double minTickTimeMs;
    double maxTickTimeMs;
    double tickTimeStdDev;
    int ticksCompleted;
    int packetsProcessed;
    double packetsPerSecond;
};

class TPSSimulator
{
public:
    TPSSimulator(int targetTPS, PacketModule& module)
        : targetTPS(targetTPS)
        , tickInterval(1000.0 / targetTPS)
        , packetModule(module)
    {
    }

    TPSMetrics RunSimulation(duration<double> simulationTime, int packetsPerTick)
    {
        std::vector<double> tickTimes;
        int totalPackets = 0;

        auto simulationStart = high_resolution_clock::now();
        auto nextTick = simulationStart;

        while (high_resolution_clock::now() - simulationStart < simulationTime)
        {
            auto tickStart = high_resolution_clock::now();

            for (int i = 0; i < packetsPerTick; ++i)
            {
                PositionPacket packet(
                    static_cast<double>(i),
                    64.0,
                    static_cast<double>(i * 2),
                    0.0f, 0.0f, true
                );

                PacketBuffer buffer = packetModule.Serialize(packet);
                buffer.Reset();
                auto restored = packetModule.Deserialize(buffer);
                totalPackets++;
            }

            auto tickEnd = high_resolution_clock::now();
            double tickTimeMs = duration<double, std::milli>(tickEnd - tickStart).count();
            tickTimes.push_back(tickTimeMs);

            nextTick += milliseconds(static_cast<int>(tickInterval));
            auto now = high_resolution_clock::now();

            if (nextTick > now)
                std::this_thread::sleep_until(nextTick);
        }

        return CalculateMetrics(tickTimes, totalPackets, simulationTime);
    }

private:
    TPSMetrics CalculateMetrics(const std::vector<double>& tickTimes, int totalPackets, duration<double> simTime)
    {
        TPSMetrics metrics{};
        metrics.ticksCompleted = static_cast<int>(tickTimes.size());
        metrics.packetsProcessed = totalPackets;

        if (tickTimes.empty())
            return metrics;

        double sum = std::accumulate(tickTimes.begin(), tickTimes.end(), 0.0);
        metrics.averageTickTimeMs = sum / tickTimes.size();

        metrics.minTickTimeMs = *std::min_element(tickTimes.begin(), tickTimes.end());
        metrics.maxTickTimeMs = *std::max_element(tickTimes.begin(), tickTimes.end());

        double sqSum = 0.0;
        for (double t : tickTimes)
            sqSum += (t - metrics.averageTickTimeMs) * (t - metrics.averageTickTimeMs);
        metrics.tickTimeStdDev = std::sqrt(sqSum / tickTimes.size());

        metrics.packetsPerSecond = totalPackets / duration<double>(simTime).count();

        return metrics;
    }

    int targetTPS;
    double tickInterval;
    PacketModule& packetModule;
};

TEST_F(NetworkingTest, GoodTPS_60_LowLoad)
{
    TPSSimulator simulator(60, packetModule);
    auto metrics = simulator.RunSimulation(500ms, 10);

    EXPECT_GE(metrics.ticksCompleted, 25);
    EXPECT_LT(metrics.averageTickTimeMs, 16.67);
    EXPECT_GT(metrics.packetsPerSecond, 500);

    std::cout << "[60 TPS, 10 packets/tick]\n"
              << "  Avg tick time: " << metrics.averageTickTimeMs << " ms\n"
              << "  Packets/sec: " << metrics.packetsPerSecond << "\n";
}

TEST_F(NetworkingTest, GoodTPS_60_MediumLoad)
{
    TPSSimulator simulator(60, packetModule);
    auto metrics = simulator.RunSimulation(500ms, 50);

    EXPECT_GE(metrics.ticksCompleted, 25);
    EXPECT_LT(metrics.averageTickTimeMs, 16.67);
    EXPECT_GT(metrics.packetsPerSecond, 2000);

    std::cout << "[60 TPS, 50 packets/tick]\n"
              << "  Avg tick time: " << metrics.averageTickTimeMs << " ms\n"
              << "  Packets/sec: " << metrics.packetsPerSecond << "\n";
}

TEST_F(NetworkingTest, GoodTPS_20_MinecraftStyle)
{
    TPSSimulator simulator(20, packetModule);
    auto metrics = simulator.RunSimulation(500ms, 100);

    EXPECT_GE(metrics.ticksCompleted, 8);
    EXPECT_LT(metrics.averageTickTimeMs, 50.0);
    EXPECT_GT(metrics.packetsPerSecond, 1500);

    std::cout << "[20 TPS Minecraft-style, 100 packets/tick]\n"
              << "  Avg tick time: " << metrics.averageTickTimeMs << " ms\n"
              << "  Packets/sec: " << metrics.packetsPerSecond << "\n";
}

TEST_F(NetworkingTest, BadTPS_10_HighLoad)
{
    TPSSimulator simulator(10, packetModule);
    auto metrics = simulator.RunSimulation(500ms, 200);

    EXPECT_GE(metrics.ticksCompleted, 4);

    std::cout << "[10 TPS degraded, 200 packets/tick]\n"
              << "  Avg tick time: " << metrics.averageTickTimeMs << " ms\n"
              << "  Max tick time: " << metrics.maxTickTimeMs << " ms\n"
              << "  Packets/sec: " << metrics.packetsPerSecond << "\n";
}

TEST_F(NetworkingTest, BadTPS_5_ExtremeLoad)
{
    TPSSimulator simulator(5, packetModule);
    auto metrics = simulator.RunSimulation(500ms, 500);

    EXPECT_GE(metrics.ticksCompleted, 2);

    std::cout << "[5 TPS extreme, 500 packets/tick]\n"
              << "  Avg tick time: " << metrics.averageTickTimeMs << " ms\n"
              << "  Max tick time: " << metrics.maxTickTimeMs << " ms\n"
              << "  Tick time std dev: " << metrics.tickTimeStdDev << " ms\n";
}

// =============================================================================
// Performance Benchmarks
// =============================================================================

TEST_F(NetworkingTest, Performance_SmallPacketThroughput)
{
    SimplePacket packet(42, "Test");
    const int iterations = 10000;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i)
    {
        PacketBuffer buffer = packetModule.Serialize(packet);
        buffer.Reset();
        auto restored = packetModule.Deserialize(buffer);
    }

    auto end = high_resolution_clock::now();
    double totalMs = duration<double, std::milli>(end - start).count();
    double packetsPerSecond = iterations / (totalMs / 1000.0);

    EXPECT_GT(packetsPerSecond, 100000);

    std::cout << "[Small Packet Throughput]\n"
              << "  " << iterations << " round-trips in " << totalMs << " ms\n"
              << "  " << packetsPerSecond << " packets/sec\n";
}

TEST_F(NetworkingTest, Performance_LargePacketThroughput)
{
    ChunkDataPacket packet(0, 0, 16384);
    const int iterations = 1000;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i)
    {
        PacketBuffer buffer = packetModule.Serialize(packet);
        buffer.Reset();
        auto restored = packetModule.Deserialize(buffer);
    }

    auto end = high_resolution_clock::now();
    double totalMs = duration<double, std::milli>(end - start).count();
    double packetsPerSecond = iterations / (totalMs / 1000.0);
    double mbPerSecond = (16384.0 * packetsPerSecond) / (1024.0 * 1024.0);

    EXPECT_GT(packetsPerSecond, 1000);

    std::cout << "[Large Packet (16KB) Throughput]\n"
              << "  " << iterations << " round-trips in " << totalMs << " ms\n"
              << "  " << packetsPerSecond << " packets/sec\n"
              << "  " << mbPerSecond << " MB/sec\n";
}

TEST_F(NetworkingTest, Performance_MixedPacketWorkload)
{
    const int iterations = 5000;
    std::vector<std::unique_ptr<IPacket>> packets;

    for (int i = 0; i < iterations; ++i)
    {
        switch (i % 4)
        {
            case 0: packets.push_back(std::make_unique<EmptyPacket>()); break;
            case 1: packets.push_back(std::make_unique<SimplePacket>(i, "msg")); break;
            case 2: packets.push_back(std::make_unique<PositionPacket>(i, 64, i*2, 0, 0, true)); break;
            case 3:
            {
                auto entity = std::make_unique<EntityUpdatePacket>();
                entity->entityId = i;
                entity->deltaX = static_cast<int16_t>(i % 100);
                entity->deltaY = 0;
                entity->deltaZ = static_cast<int16_t>(i % 100);
                entity->hasVelocity = (i % 2 == 0);
                packets.push_back(std::move(entity));
                break;
            }
        }
    }

    auto start = high_resolution_clock::now();

    for (const auto& packet : packets)
    {
        PacketBuffer buffer = packetModule.Serialize(*packet);
        buffer.Reset();
        auto restored = packetModule.Deserialize(buffer);
    }

    auto end = high_resolution_clock::now();
    double totalMs = duration<double, std::milli>(end - start).count();
    double packetsPerSecond = iterations / (totalMs / 1000.0);

    EXPECT_GT(packetsPerSecond, 50000);

    std::cout << "[Mixed Workload]\n"
              << "  " << iterations << " mixed packets in " << totalMs << " ms\n"
              << "  " << packetsPerSecond << " packets/sec\n";
}

TEST_F(NetworkingTest, Performance_SerializationLatency)
{
    PositionPacket packet(100.5, 64.0, -200.25, 45.0f, -30.0f, true);

    double avgTime = MeasureSerializationTimeMs(packet, 10000);

    EXPECT_LT(avgTime, 0.01);

    std::cout << "[Serialization Latency]\n"
              << "  Average: " << (avgTime * 1000.0) << " microseconds\n";
}

TEST_F(NetworkingTest, Performance_DeserializationLatency)
{
    PositionPacket packet(100.5, 64.0, -200.25, 45.0f, -30.0f, true);
    PacketBuffer buffer = packetModule.Serialize(packet);

    double avgTime = MeasureDeserializationTimeMs<PositionPacket>(buffer, 10000);

    EXPECT_LT(avgTime, 0.01);

    std::cout << "[Deserialization Latency]\n"
              << "  Average: " << (avgTime * 1000.0) << " microseconds\n";
}

// =============================================================================
// Stress Tests
// =============================================================================

TEST_F(NetworkingTest, Stress_HighFrequencyBurst)
{
    const int burstSize = 1000;
    const int bursts = 10;
    std::vector<double> burstTimes;

    for (int burst = 0; burst < bursts; ++burst)
    {
        auto start = high_resolution_clock::now();

        for (int i = 0; i < burstSize; ++i)
        {
            PositionPacket packet(i, 64, i * 2, 0, 0, true);
            PacketBuffer buffer = packetModule.Serialize(packet);
            buffer.Reset();
            auto restored = packetModule.Deserialize(buffer);
        }

        auto end = high_resolution_clock::now();
        burstTimes.push_back(duration<double, std::milli>(end - start).count());
    }

    double avgBurstTime = std::accumulate(burstTimes.begin(), burstTimes.end(), 0.0) / bursts;
    double maxBurstTime = *std::max_element(burstTimes.begin(), burstTimes.end());

    EXPECT_LT(maxBurstTime / avgBurstTime, 2.0);

    std::cout << "[High Frequency Burst]\n"
              << "  Avg burst time: " << avgBurstTime << " ms\n"
              << "  Max burst time: " << maxBurstTime << " ms\n"
              << "  Consistency ratio: " << (maxBurstTime / avgBurstTime) << "\n";
}

TEST_F(NetworkingTest, Stress_SustainedLoad)
{
    const auto testDuration = 2s;
    int packetsProcessed = 0;
    std::vector<double> latencies;

    auto start = high_resolution_clock::now();

    while (high_resolution_clock::now() - start < testDuration)
    {
        auto packetStart = high_resolution_clock::now();

        PositionPacket packet(packetsProcessed, 64, packetsProcessed * 2, 0, 0, true);
        PacketBuffer buffer = packetModule.Serialize(packet);
        buffer.Reset();
        auto restored = packetModule.Deserialize(buffer);

        auto packetEnd = high_resolution_clock::now();
        latencies.push_back(duration<double, std::micro>(packetEnd - packetStart).count());

        packetsProcessed++;
    }

    double avgLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();

    std::sort(latencies.begin(), latencies.end());
    double p99Latency = latencies[static_cast<size_t>(latencies.size() * 0.99)];
    double p999Latency = latencies[static_cast<size_t>(latencies.size() * 0.999)];

    double packetsPerSecond = packetsProcessed / 2.0;

    EXPECT_GT(packetsPerSecond, 100000);
    EXPECT_LT(p99Latency, 100);

    std::cout << "[Sustained Load - 2 seconds]\n"
              << "  Total packets: " << packetsProcessed << "\n"
              << "  Packets/sec: " << packetsPerSecond << "\n"
              << "  Avg latency: " << avgLatency << " us\n"
              << "  P99 latency: " << p99Latency << " us\n"
              << "  P99.9 latency: " << p999Latency << " us\n";
}

TEST_F(NetworkingTest, Stress_MemoryStability)
{
    const int iterations = 100000;

    for (int i = 0; i < iterations; ++i)
    {
        ChunkDataPacket packet(i % 100, i % 100, 1024);
        PacketBuffer buffer = packetModule.Serialize(packet);
        buffer.Reset();
        auto restored = packetModule.Deserialize(buffer);
    }

    SUCCEED();
}

// =============================================================================
// Edge Cases and Error Handling
// =============================================================================

TEST_F(NetworkingTest, EdgeCase_EmptyStringPacket)
{
    SimplePacket original(0, "");
    PacketBuffer buffer = packetModule.Serialize(original);
    buffer.Reset();

    auto restored = packetModule.Deserialize(buffer);
    auto* simplePacket = dynamic_cast<SimplePacket*>(restored.get());

    ASSERT_NE(simplePacket, nullptr);
    EXPECT_EQ(simplePacket->message, "");
}

TEST_F(NetworkingTest, EdgeCase_MaxStringLength)
{
    std::string longString(32767, 'A');
    SimplePacket original(0, longString);
    PacketBuffer buffer = packetModule.Serialize(original);
    buffer.Reset();

    auto restored = packetModule.Deserialize(buffer);
    auto* simplePacket = dynamic_cast<SimplePacket*>(restored.get());

    ASSERT_NE(simplePacket, nullptr);
    EXPECT_EQ(simplePacket->message.length(), 32767u);
}

TEST_F(NetworkingTest, EdgeCase_UnicodeString)
{
    std::string unicodeStr = "Hello \xe4\xb8\x96\xe7\x95\x8c \xf0\x9f\x98\x80";
    SimplePacket original(0, unicodeStr);
    PacketBuffer buffer = packetModule.Serialize(original);
    buffer.Reset();

    auto restored = packetModule.Deserialize(buffer);
    auto* simplePacket = dynamic_cast<SimplePacket*>(restored.get());

    ASSERT_NE(simplePacket, nullptr);
    EXPECT_EQ(simplePacket->message, original.message);
}

TEST_F(NetworkingTest, EdgeCase_ExtremeNumericValues)
{
    PositionPacket original(
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::min(),
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::lowest(),
        true
    );

    PacketBuffer buffer = packetModule.Serialize(original);
    buffer.Reset();

    auto restored = packetModule.Deserialize(buffer);
    auto* posPacket = dynamic_cast<PositionPacket*>(restored.get());

    ASSERT_NE(posPacket, nullptr);
    EXPECT_DOUBLE_EQ(posPacket->x, std::numeric_limits<double>::max());
    EXPECT_DOUBLE_EQ(posPacket->y, std::numeric_limits<double>::min());
    EXPECT_FLOAT_EQ(posPacket->yaw, std::numeric_limits<float>::max());
}

TEST_F(NetworkingTest, EdgeCase_NaNAndInfinity)
{
    PositionPacket original(
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::quiet_NaN(),
        false
    );

    PacketBuffer buffer = packetModule.Serialize(original);
    buffer.Reset();

    auto restored = packetModule.Deserialize(buffer);
    auto* posPacket = dynamic_cast<PositionPacket*>(restored.get());

    ASSERT_NE(posPacket, nullptr);
    EXPECT_TRUE(std::isinf(posPacket->x));
    EXPECT_TRUE(std::isinf(posPacket->y));
    EXPECT_TRUE(std::isnan(posPacket->z));
    EXPECT_TRUE(std::isinf(posPacket->yaw));
    EXPECT_TRUE(std::isnan(posPacket->pitch));
}

TEST_F(NetworkingTest, EdgeCase_ZeroSizeChunk)
{
    ChunkDataPacket original(0, 0, 0);
    PacketBuffer buffer = packetModule.Serialize(original);
    buffer.Reset();

    auto restored = packetModule.Deserialize(buffer);
    auto* chunkPacket = dynamic_cast<ChunkDataPacket*>(restored.get());

    ASSERT_NE(chunkPacket, nullptr);
    EXPECT_TRUE(chunkPacket->blockData.empty());
}

TEST_F(NetworkingTest, EdgeCase_VarintBoundaries)
{
    std::vector<int32_t> testValues = {
        0, 1, 127, 128, 255, 256,
        16383, 16384,
        2097151, 2097152,
        268435455, 268435456,
        std::numeric_limits<int32_t>::max(),
        -1, -128, -129,
        std::numeric_limits<int32_t>::min()
    };

    for (int32_t value : testValues)
    {
        PacketBuffer buffer = PacketBuffer::Allocate();
        buffer.WriteVarint(value);
        buffer.Reset();
        int32_t restored = buffer.ReadVarint();
        EXPECT_EQ(restored, value) << "Failed for value: " << value;
    }
}

TEST_F(NetworkingTest, ErrorHandling_BufferUnderflow)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteInt32(42);
    buffer.Reset();

    buffer.ReadInt32();
    EXPECT_THROW(buffer.ReadInt32(), std::runtime_error);
}

TEST_F(NetworkingTest, ErrorHandling_InvalidPacketId)
{
    PacketBuffer buffer = PacketBuffer::Allocate();
    buffer.WriteVarint(0xFF);
    buffer.Reset();

    auto packet = packetModule.Deserialize(buffer);
    EXPECT_EQ(packet, nullptr);
}

// =============================================================================
// Concurrent Operations (Thread Safety)
// =============================================================================

TEST_F(NetworkingTest, Concurrent_ParallelSerialization)
{
    const int threadsCount = 4;
    const int packetsPerThread = 10000;
    std::atomic<int> successCount{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < threadsCount; ++t)
    {
        threads.emplace_back([&, t]()
        {
            for (int i = 0; i < packetsPerThread; ++i)
            {
                PositionPacket packet(t * 1000 + i, 64, i * 2, 0, 0, true);
                PacketBuffer buffer = PacketBuffer::Allocate();
                buffer.WriteVarint(0x02);
                packet.Write(buffer);
                buffer.Reset();

                buffer.ReadVarint();
                PositionPacket restored;
                restored.Read(buffer);

                if (static_cast<int>(restored.x) == t * 1000 + i)
                    successCount++;
            }
        });
    }

    for (auto& thread : threads)
        thread.join();

    EXPECT_EQ(successCount.load(), threadsCount * packetsPerThread);
}

TEST_F(NetworkingTest, Concurrent_ProducerConsumer)
{
    std::queue<PacketBuffer> packetQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> producerDone{false};
    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};
    const int totalPackets = 10000;

    std::thread producer([&]()
    {
        for (int i = 0; i < totalPackets; ++i)
        {
            PositionPacket packet(i, 64, i * 2, 0, 0, true);
            PacketBuffer buffer = PacketBuffer::Allocate();
            buffer.WriteVarint(0x02);
            packet.Write(buffer);

            {
                std::lock_guard<std::mutex> lock(queueMutex);
                packetQueue.push(std::move(buffer));
                produced++;
            }
            cv.notify_one();
        }
        producerDone = true;
        cv.notify_all();
    });

    std::thread consumer([&]()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [&]() { return !packetQueue.empty() || producerDone.load(); });

            while (!packetQueue.empty())
            {
                PacketBuffer buffer = std::move(packetQueue.front());
                packetQueue.pop();
                lock.unlock();

                buffer.Reset();
                buffer.ReadVarint();
                PositionPacket restored;
                restored.Read(buffer);
                consumed++;

                lock.lock();
            }

            if (producerDone && packetQueue.empty())
                break;
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(produced.load(), totalPackets);
    EXPECT_EQ(consumed.load(), totalPackets);
}

// =============================================================================
// Industry Standard Compliance Tests
// =============================================================================

TEST_F(NetworkingTest, IndustryStandard_LatencyUnder1ms)
{
    PositionPacket packet(100.5, 64.0, -200.25, 45.0f, -30.0f, true);
    double avgRoundTrip = MeasureRoundTripTimeMs(packet, 10000);

    EXPECT_LT(avgRoundTrip, 1.0);

    std::cout << "[Industry Standard: Latency]\n"
              << "  Round-trip: " << (avgRoundTrip * 1000.0) << " microseconds\n"
              << "  Status: " << (avgRoundTrip < 0.1 ? "EXCELLENT" :
                                   avgRoundTrip < 0.5 ? "GOOD" : "ACCEPTABLE") << "\n";
}

TEST_F(NetworkingTest, IndustryStandard_ThroughputOver100kPPS)
{
    SimplePacket packet(42, "Test message");
    const int iterations = 100000;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i)
    {
        PacketBuffer buffer = packetModule.Serialize(packet);
        buffer.Reset();
        auto restored = packetModule.Deserialize(buffer);
    }

    auto end = high_resolution_clock::now();
    double seconds = duration<double>(end - start).count();
    double pps = iterations / seconds;

    EXPECT_GT(pps, 100000);

    std::cout << "[Industry Standard: Throughput]\n"
              << "  " << static_cast<int>(pps) << " packets/sec\n"
              << "  Status: " << (pps > 500000 ? "EXCELLENT" :
                                   pps > 200000 ? "GOOD" : "ACCEPTABLE") << "\n";
}

TEST_F(NetworkingTest, IndustryStandard_ConsistentPerformance)
{
    const int samples = 100;
    std::vector<double> throughputs;

    for (int s = 0; s < samples; ++s)
    {
        const int iterations = 1000;
        auto start = high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i)
        {
            PositionPacket packet(i, 64, i * 2, 0, 0, true);
            PacketBuffer buffer = packetModule.Serialize(packet);
            buffer.Reset();
            auto restored = packetModule.Deserialize(buffer);
        }

        auto end = high_resolution_clock::now();
        double seconds = duration<double>(end - start).count();
        throughputs.push_back(iterations / seconds);
    }

    double avgThroughput = std::accumulate(throughputs.begin(), throughputs.end(), 0.0) / samples;
    double minThroughput = *std::min_element(throughputs.begin(), throughputs.end());
    double maxThroughput = *std::max_element(throughputs.begin(), throughputs.end());

    double variance = maxThroughput / minThroughput;

    EXPECT_LT(variance, 3.0);

    std::cout << "[Industry Standard: Consistency]\n"
              << "  Avg: " << static_cast<int>(avgThroughput) << " pps\n"
              << "  Min: " << static_cast<int>(minThroughput) << " pps\n"
              << "  Max: " << static_cast<int>(maxThroughput) << " pps\n"
              << "  Variance ratio: " << variance << "\n"
              << "  Status: " << (variance < 1.5 ? "EXCELLENT" :
                                   variance < 2.5 ? "GOOD" : "NEEDS ATTENTION") << "\n";
}

TEST_F(NetworkingTest, IndustryStandard_60TPSBudget)
{
    const double tickBudgetMs = 16.67;
    const int packetsPerTick = 100;
    const int ticks = 60;
    int ticksWithinBudget = 0;

    for (int tick = 0; tick < ticks; ++tick)
    {
        auto start = high_resolution_clock::now();

        for (int i = 0; i < packetsPerTick; ++i)
        {
            PositionPacket packet(i, 64, i * 2, 0, 0, true);
            PacketBuffer buffer = packetModule.Serialize(packet);
            buffer.Reset();
            auto restored = packetModule.Deserialize(buffer);
        }

        auto end = high_resolution_clock::now();
        double tickTimeMs = duration<double, std::milli>(end - start).count();

        if (tickTimeMs < tickBudgetMs)
            ticksWithinBudget++;
    }

    double successRate = static_cast<double>(ticksWithinBudget) / ticks * 100.0;

    EXPECT_GT(successRate, 95.0);

    std::cout << "[Industry Standard: 60 TPS Budget]\n"
              << "  Ticks within 16.67ms budget: " << ticksWithinBudget << "/" << ticks << "\n"
              << "  Success rate: " << successRate << "%\n"
              << "  Status: " << (successRate >= 99 ? "EXCELLENT" :
                                   successRate >= 95 ? "GOOD" : "NEEDS OPTIMIZATION") << "\n";
}

TEST_F(NetworkingTest, IndustryStandard_20TPSBudget)
{
    const double tickBudgetMs = 50.0;
    const int packetsPerTick = 500;
    const int ticks = 20;
    int ticksWithinBudget = 0;

    for (int tick = 0; tick < ticks; ++tick)
    {
        auto start = high_resolution_clock::now();

        for (int i = 0; i < packetsPerTick; ++i)
        {
            EntityUpdatePacket packet;
            packet.entityId = i;
            packet.deltaX = static_cast<int16_t>(i % 100);
            packet.deltaY = static_cast<int16_t>((i / 100) % 100);
            packet.deltaZ = static_cast<int16_t>(i % 50);
            packet.hasVelocity = (i % 3 == 0);

            PacketBuffer buffer = packetModule.Serialize(packet);
            buffer.Reset();
            auto restored = packetModule.Deserialize(buffer);
        }

        auto end = high_resolution_clock::now();
        double tickTimeMs = duration<double, std::milli>(end - start).count();

        if (tickTimeMs < tickBudgetMs)
            ticksWithinBudget++;
    }

    double successRate = static_cast<double>(ticksWithinBudget) / ticks * 100.0;

    EXPECT_GT(successRate, 95.0);

    std::cout << "[Industry Standard: 20 TPS Budget (Minecraft-style)]\n"
              << "  Ticks within 50ms budget: " << ticksWithinBudget << "/" << ticks << "\n"
              << "  Success rate: " << successRate << "%\n"
              << "  Status: " << (successRate >= 99 ? "EXCELLENT" :
                                   successRate >= 95 ? "GOOD" : "NEEDS OPTIMIZATION") << "\n";
}
