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

        std::vector<Scene::MapbinMaterial> materials;
        std::vector<Scene::MapbinGroup> groups;

        void Write(PacketBuffer& buffer) const override
        {
            buffer.WriteInt32(static_cast<int32_t>(materials.size()));

            for (const auto& mat : materials)
            {
                buffer.WriteInt32(mat.materialId);
                buffer.WriteFloat(mat.normalStrength);
                buffer.WriteFloat(mat.roughness);
                buffer.WriteFloat(mat.metallic);
                buffer.WriteFloat(mat.specularStrength);
                buffer.WriteFloat(mat.detailScale);
                buffer.WriteFloat(mat.emissionStrength);
                buffer.WriteFloat(mat.aoStrength);
                buffer.WriteInt32(static_cast<int32_t>(mat.textureSlots.size()));

                for (const auto& slot : mat.textureSlots)
                {
                    buffer.WriteInt32(static_cast<int32_t>(slot.slotType));
                    buffer.WriteInt32(static_cast<int32_t>(slot.width));
                    buffer.WriteInt32(static_cast<int32_t>(slot.height));
                    buffer.WriteInt32(static_cast<int32_t>(slot.wrapS));
                    buffer.WriteInt32(static_cast<int32_t>(slot.wrapT));
                    buffer.WriteInt32(static_cast<int32_t>(slot.minFilter));
                    buffer.WriteInt32(static_cast<int32_t>(slot.magFilter));
                    buffer.WriteInt32(static_cast<int32_t>(slot.pixelData.size()));

                    for (uint8_t byte : slot.pixelData)
                        buffer.WriteByte(static_cast<std::byte>(byte));
                }
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
            int32_t materialCount = buffer.ReadInt32();
            materials.resize(materialCount);

            for (auto& mat : materials)
            {
                mat.materialId = buffer.ReadInt32();
                mat.normalStrength = buffer.ReadFloat();
                mat.roughness = buffer.ReadFloat();
                mat.metallic = buffer.ReadFloat();
                mat.specularStrength = buffer.ReadFloat();
                mat.detailScale = buffer.ReadFloat();
                mat.emissionStrength = buffer.ReadFloat();
                mat.aoStrength = buffer.ReadFloat();
                int32_t slotCount = buffer.ReadInt32();
                mat.textureSlots.resize(slotCount);

                for (auto& slot : mat.textureSlots)
                {
                    slot.slotType = static_cast<Scene::TextureSlotType>(buffer.ReadInt32());
                    slot.width = static_cast<uint32_t>(buffer.ReadInt32());
                    slot.height = static_cast<uint32_t>(buffer.ReadInt32());
                    slot.wrapS = static_cast<uint32_t>(buffer.ReadInt32());
                    slot.wrapT = static_cast<uint32_t>(buffer.ReadInt32());
                    slot.minFilter = static_cast<uint32_t>(buffer.ReadInt32());
                    slot.magFilter = static_cast<uint32_t>(buffer.ReadInt32());
                    int32_t pixelSize = buffer.ReadInt32();
                    slot.pixelData.resize(pixelSize);

                    for (uint8_t& byte : slot.pixelData)
                        byte = static_cast<uint8_t>(buffer.ReadByte());
                }
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
