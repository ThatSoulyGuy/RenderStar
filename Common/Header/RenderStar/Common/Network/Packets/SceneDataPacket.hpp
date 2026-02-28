#pragma once

#include "RenderStar/Common/Network/IPacket.hpp"
#include "RenderStar/Common/Network/PacketBuffer.hpp"
#include "RenderStar/Common/Scene/MapbinLoader.hpp"
#include <cstdint>
#include <vector>

namespace RenderStar::Common::Network::Packets
{
    class SceneDataPacket final : public IPacket
    {
    public:

        std::vector<Scene::MapbinTexture> textures;
        std::vector<Scene::MapbinGroup> groups;

        void Write(PacketBuffer& buffer) const override
        {
            buffer.WriteInt32(static_cast<int32_t>(textures.size()));

            for (const auto& tex : textures)
            {
                buffer.WriteInt32(tex.materialId);
                buffer.WriteInt32(static_cast<int32_t>(tex.width));
                buffer.WriteInt32(static_cast<int32_t>(tex.height));
                buffer.WriteInt32(static_cast<int32_t>(tex.wrapS));
                buffer.WriteInt32(static_cast<int32_t>(tex.wrapT));
                buffer.WriteInt32(static_cast<int32_t>(tex.minFilter));
                buffer.WriteInt32(static_cast<int32_t>(tex.magFilter));
                buffer.WriteInt32(static_cast<int32_t>(tex.pixelData.size()));

                for (uint8_t byte : tex.pixelData)
                    buffer.WriteByte(static_cast<std::byte>(byte));
            }

            buffer.WriteInt32(static_cast<int32_t>(groups.size()));

            for (const auto& group : groups)
            {
                buffer.WriteInt32(group.materialId);
                buffer.WriteInt32(group.vertexCount);
                buffer.WriteInt32(static_cast<int32_t>(group.indices.size()));

                for (float f : group.vertexData)
                    buffer.WriteFloat(f);

                for (uint32_t idx : group.indices)
                    buffer.WriteInt32(static_cast<int32_t>(idx));
            }
        }

        void Read(PacketBuffer& buffer) override
        {
            int32_t textureCount = buffer.ReadInt32();
            textures.resize(textureCount);

            for (auto& tex : textures)
            {
                tex.materialId = buffer.ReadInt32();
                tex.width = static_cast<uint32_t>(buffer.ReadInt32());
                tex.height = static_cast<uint32_t>(buffer.ReadInt32());
                tex.wrapS = static_cast<uint32_t>(buffer.ReadInt32());
                tex.wrapT = static_cast<uint32_t>(buffer.ReadInt32());
                tex.minFilter = static_cast<uint32_t>(buffer.ReadInt32());
                tex.magFilter = static_cast<uint32_t>(buffer.ReadInt32());
                int32_t pixelSize = buffer.ReadInt32();
                tex.pixelData.resize(pixelSize);

                for (uint8_t& byte : tex.pixelData)
                    byte = static_cast<uint8_t>(buffer.ReadByte());
            }

            int32_t groupCount = buffer.ReadInt32();
            groups.resize(groupCount);

            for (auto& group : groups)
            {
                group.materialId = buffer.ReadInt32();
                group.vertexCount = buffer.ReadInt32();
                int32_t indexCount = buffer.ReadInt32();

                group.vertexData.resize(static_cast<size_t>(group.vertexCount) * 8);

                for (float& f : group.vertexData)
                    f = buffer.ReadFloat();

                group.indices.resize(indexCount);

                for (uint32_t& idx : group.indices)
                    idx = static_cast<uint32_t>(buffer.ReadInt32());
            }
        }
    };
}
