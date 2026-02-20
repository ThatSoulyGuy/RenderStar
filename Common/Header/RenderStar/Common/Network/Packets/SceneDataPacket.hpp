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

        std::vector<Scene::MapbinGroup> groups;

        void Write(PacketBuffer& buffer) const override
        {
            buffer.WriteInt32(static_cast<int32_t>(groups.size()));

            for (const auto& group : groups)
            {
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
            int32_t groupCount = buffer.ReadInt32();
            groups.resize(groupCount);

            for (auto& group : groups)
            {
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
