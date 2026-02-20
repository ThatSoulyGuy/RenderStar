#include "RenderStar/Common/Scene/MapbinLoader.hpp"
#include <algorithm>
#include <cstring>

namespace RenderStar::Common::Scene
{
    namespace
    {
        uint32_t ReadUint32(const uint8_t* ptr)
        {
            uint32_t value;
            std::memcpy(&value, ptr, sizeof(uint32_t));
            return value;
        }

        float ReadFloat(const uint8_t* ptr)
        {
            float value;
            std::memcpy(&value, ptr, sizeof(float));
            return value;
        }
    }

    std::optional<MapbinScene> MapbinLoader::Load(std::span<const uint8_t> data)
    {
        if (data.size() < HEADER_SIZE)
            return std::nullopt;

        const uint8_t* ptr = data.data();
        const uint8_t* end = ptr + data.size();

        uint32_t magic = ReadUint32(ptr);

        if (magic != MAGIC)
            return std::nullopt;

        uint32_t version = ReadUint32(ptr + 4);

        if (version != VERSION)
            return std::nullopt;

        uint32_t textureCount = ReadUint32(ptr + 8);
        uint32_t groupCount = ReadUint32(ptr + 12);
        ptr += HEADER_SIZE;

        for (uint32_t i = 0; i < textureCount; ++i)
        {
            if (ptr + TEXTURE_HEADER_SIZE > end)
                return std::nullopt;

            uint32_t pixelDataSize = ReadUint32(ptr + 28);
            ptr += TEXTURE_HEADER_SIZE;

            if (ptr + pixelDataSize > end)
                return std::nullopt;

            ptr += pixelDataSize;
        }

        MapbinScene scene;
        scene.groups.reserve(groupCount);

        for (uint32_t i = 0; i < groupCount; ++i)
        {
            if (ptr + GROUP_HEADER_SIZE > end)
                return std::nullopt;

            ptr += 4;
            uint32_t vertexCount = ReadUint32(ptr);
            ptr += 4;
            uint32_t indexCount = ReadUint32(ptr);
            ptr += 4;

            size_t vertexBytes = static_cast<size_t>(vertexCount) * VERTEX_SIZE;
            size_t indexBytes = static_cast<size_t>(indexCount) * sizeof(uint32_t);

            if (ptr + vertexBytes + indexBytes > end)
                return std::nullopt;

            MapbinGroup group;
            group.vertexCount = static_cast<int32_t>(vertexCount);
            group.vertexData.resize(static_cast<size_t>(vertexCount) * 8);

            for (uint32_t v = 0; v < vertexCount; ++v)
            {
                size_t offset = static_cast<size_t>(v) * 8;

                group.vertexData[offset + 0] = ReadFloat(ptr + 0);
                group.vertexData[offset + 1] = ReadFloat(ptr + 4);
                group.vertexData[offset + 2] = ReadFloat(ptr + 8);

                group.vertexData[offset + 3] = ReadFloat(ptr + 12) * 0.5f + 0.5f;
                group.vertexData[offset + 4] = ReadFloat(ptr + 16) * 0.5f + 0.5f;
                group.vertexData[offset + 5] = ReadFloat(ptr + 20) * 0.5f + 0.5f;

                group.vertexData[offset + 6] = ReadFloat(ptr + 24);
                group.vertexData[offset + 7] = ReadFloat(ptr + 28);

                ptr += VERTEX_SIZE;
            }

            group.indices.resize(indexCount);

            for (uint32_t j = 0; j < indexCount; ++j)
            {
                group.indices[j] = ReadUint32(ptr);
                ptr += sizeof(uint32_t);
            }

            for (uint32_t j = 0; j + 2 < indexCount; j += 3)
                std::swap(group.indices[j + 1], group.indices[j + 2]);

            scene.groups.push_back(std::move(group));
        }

        return scene;
    }
}
