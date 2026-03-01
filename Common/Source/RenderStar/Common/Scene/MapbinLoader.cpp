#include "RenderStar/Common/Scene/MapbinLoader.hpp"
#include <algorithm>
#include <cmath>
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
        uint32_t count1 = ReadUint32(ptr + 8);
        uint32_t count2 = ReadUint32(ptr + 12);
        ptr += HEADER_SIZE;

        if (version == VERSION_2)
            return LoadV2(ptr, end, count1, count2);

        if (version == VERSION_3)
            return LoadV3(ptr, end, count1, count2);

        if (version == VERSION_4)
            return LoadV4(ptr, end, count1, count2);

        if (version == VERSION_5)
            return LoadV5(ptr, end, count1, count2);

        return std::nullopt;
    }

    std::optional<MapbinScene> MapbinLoader::LoadV2(const uint8_t* ptr, const uint8_t* end,
        uint32_t textureCount, uint32_t groupCount)
    {
        MapbinScene scene;
        scene.materials.reserve(textureCount);

        for (uint32_t i = 0; i < textureCount; ++i)
        {
            if (ptr + V2_TEXTURE_HEADER_SIZE > end)
                return std::nullopt;

            MapbinMaterial material;
            material.materialId = static_cast<int32_t>(ReadUint32(ptr));

            MapbinTextureSlot slot;
            slot.slotType = TextureSlotType::BASE_COLOR;
            slot.width = ReadUint32(ptr + 4);
            slot.height = ReadUint32(ptr + 8);
            slot.wrapS = ReadUint32(ptr + 12);
            slot.wrapT = ReadUint32(ptr + 16);
            slot.minFilter = ReadUint32(ptr + 20);
            slot.magFilter = ReadUint32(ptr + 24);
            uint32_t pixelDataSize = ReadUint32(ptr + 28);
            ptr += V2_TEXTURE_HEADER_SIZE;

            if (ptr + pixelDataSize > end)
                return std::nullopt;

            slot.pixelData.assign(ptr, ptr + pixelDataSize);
            ptr += pixelDataSize;

            material.textureSlots.push_back(std::move(slot));
            scene.materials.push_back(std::move(material));
        }

        scene.groups.reserve(groupCount);

        for (uint32_t i = 0; i < groupCount; ++i)
        {
            if (ptr + GROUP_HEADER_SIZE > end)
                return std::nullopt;

            uint32_t materialId = ReadUint32(ptr);
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
            group.materialId = static_cast<int32_t>(materialId);
            group.vertexCount = static_cast<int32_t>(vertexCount);
            group.vertexData.resize(static_cast<size_t>(vertexCount) * 8);

            for (uint32_t v = 0; v < vertexCount; ++v)
            {
                size_t offset = static_cast<size_t>(v) * 8;

                group.vertexData[offset + 0] = ReadFloat(ptr + 0);
                group.vertexData[offset + 1] = ReadFloat(ptr + 4);
                group.vertexData[offset + 2] = ReadFloat(ptr + 8);

                group.vertexData[offset + 3] = 0.0f;
                group.vertexData[offset + 4] = 1.0f;
                group.vertexData[offset + 5] = 0.0f;

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

            ComputeNormalsForGroup(group);

            scene.groups.push_back(std::move(group));
        }

        return scene;
    }

    bool MapbinLoader::ParseMaterialsAndGroups(const uint8_t*& ptr, const uint8_t* end,
        uint32_t materialCount, uint32_t groupCount, MapbinScene& scene)
    {
        scene.materials.reserve(materialCount);

        for (uint32_t i = 0; i < materialCount; ++i)
        {
            if (ptr + V3_MATERIAL_SCALAR_SIZE + 4 > end)
                return false;

            MapbinMaterial material;
            material.materialId = static_cast<int32_t>(ReadUint32(ptr));
            material.normalStrength = ReadFloat(ptr + 4);
            material.roughness = ReadFloat(ptr + 8);
            material.metallic = ReadFloat(ptr + 12);
            material.specularStrength = ReadFloat(ptr + 16);
            material.detailScale = ReadFloat(ptr + 20);
            material.emissionStrength = ReadFloat(ptr + 24);
            material.aoStrength = ReadFloat(ptr + 28);
            uint32_t slotCount = ReadUint32(ptr + 32);
            ptr += V3_MATERIAL_SCALAR_SIZE + 4;

            material.textureSlots.reserve(slotCount);

            for (uint32_t s = 0; s < slotCount; ++s)
            {
                if (ptr + V3_SLOT_HEADER_SIZE > end)
                    return false;

                MapbinTextureSlot slot;
                slot.slotType = static_cast<TextureSlotType>(ReadUint32(ptr));
                slot.width = ReadUint32(ptr + 4);
                slot.height = ReadUint32(ptr + 8);
                slot.wrapS = ReadUint32(ptr + 12);
                slot.wrapT = ReadUint32(ptr + 16);
                slot.minFilter = ReadUint32(ptr + 20);
                slot.magFilter = ReadUint32(ptr + 24);
                uint32_t pixelDataSize = ReadUint32(ptr + 28);
                ptr += V3_SLOT_HEADER_SIZE;

                if (ptr + pixelDataSize > end)
                    return false;

                slot.pixelData.assign(ptr, ptr + pixelDataSize);
                ptr += pixelDataSize;

                material.textureSlots.push_back(std::move(slot));
            }

            scene.materials.push_back(std::move(material));
        }

        scene.groups.reserve(groupCount);

        for (uint32_t i = 0; i < groupCount; ++i)
        {
            if (ptr + GROUP_HEADER_SIZE > end)
                return false;

            uint32_t materialId = ReadUint32(ptr);
            ptr += 4;
            uint32_t vertexCount = ReadUint32(ptr);
            ptr += 4;
            uint32_t indexCount = ReadUint32(ptr);
            ptr += 4;

            size_t vertexBytes = static_cast<size_t>(vertexCount) * VERTEX_SIZE;
            size_t indexBytes = static_cast<size_t>(indexCount) * sizeof(uint32_t);

            if (ptr + vertexBytes + indexBytes > end)
                return false;

            MapbinGroup group;
            group.materialId = static_cast<int32_t>(materialId);
            group.vertexCount = static_cast<int32_t>(vertexCount);
            group.vertexData.resize(static_cast<size_t>(vertexCount) * 8);

            for (uint32_t v = 0; v < vertexCount; ++v)
            {
                size_t offset = static_cast<size_t>(v) * 8;

                group.vertexData[offset + 0] = ReadFloat(ptr + 0);
                group.vertexData[offset + 1] = ReadFloat(ptr + 4);
                group.vertexData[offset + 2] = ReadFloat(ptr + 8);
                group.vertexData[offset + 3] = ReadFloat(ptr + 12);
                group.vertexData[offset + 4] = ReadFloat(ptr + 16);
                group.vertexData[offset + 5] = ReadFloat(ptr + 20);
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

            scene.groups.push_back(std::move(group));
        }

        return true;
    }

    std::optional<MapbinScene> MapbinLoader::LoadV3(const uint8_t* ptr, const uint8_t* end,
        uint32_t materialCount, uint32_t groupCount)
    {
        MapbinScene scene;

        if (!ParseMaterialsAndGroups(ptr, end, materialCount, groupCount, scene))
            return std::nullopt;

        return scene;
    }

    std::optional<MapbinScene> MapbinLoader::LoadV4(const uint8_t* ptr, const uint8_t* end,
        uint32_t materialCount, uint32_t groupCount)
    {
        MapbinScene scene;

        if (!ParseMaterialsAndGroups(ptr, end, materialCount, groupCount, scene))
            return std::nullopt;

        if (!ParseGameObjectsV4(ptr, end, scene))
            return std::nullopt;

        return scene;
    }

    std::optional<MapbinScene> MapbinLoader::LoadV5(const uint8_t* ptr, const uint8_t* end,
        uint32_t materialCount, uint32_t groupCount)
    {
        MapbinScene scene;

        if (!ParseMaterialsAndGroups(ptr, end, materialCount, groupCount, scene))
            return std::nullopt;

        if (!ParseGameObjectsV5(ptr, end, scene))
            return std::nullopt;

        return scene;
    }

    bool MapbinLoader::ParseGameObjectsV4(const uint8_t*& ptr, const uint8_t* end, MapbinScene& scene)
    {
        if (ptr + 4 > end)
            return false;

        uint32_t count = ReadUint32(ptr);
        ptr += 4;

        scene.gameObjects.reserve(count);

        for (uint32_t i = 0; i < count; ++i)
        {
            if (ptr + V4_GAME_OBJECT_SIZE > end)
                return false;

            MapbinGameObject obj;
            obj.type = static_cast<GameObjectType>(ReadUint32(ptr));
            obj.posX = ReadFloat(ptr + 4);
            obj.posY = ReadFloat(ptr + 8);
            obj.posZ = ReadFloat(ptr + 12);
            ptr += V4_GAME_OBJECT_SIZE;

            scene.gameObjects.push_back(obj);
        }

        return true;
    }

    bool MapbinLoader::ParseGameObjectsV5(const uint8_t*& ptr, const uint8_t* end, MapbinScene& scene)
    {
        if (ptr + 4 > end)
            return false;

        uint32_t count = ReadUint32(ptr);
        ptr += 4;

        scene.gameObjects.reserve(count);

        for (uint32_t i = 0; i < count; ++i)
        {
            if (ptr + V5_GAME_OBJECT_COMMON_SIZE > end)
                return false;

            MapbinGameObject obj;
            obj.type = static_cast<GameObjectType>(ReadUint32(ptr));
            obj.posX = ReadFloat(ptr + 4);
            obj.posY = ReadFloat(ptr + 8);
            obj.posZ = ReadFloat(ptr + 12);
            obj.rotX = ReadFloat(ptr + 16);
            obj.rotY = ReadFloat(ptr + 20);
            obj.rotZ = ReadFloat(ptr + 24);
            ptr += V5_GAME_OBJECT_COMMON_SIZE;

            if (static_cast<uint32_t>(obj.type) >= 1)
            {
                if (ptr + V5_LIGHT_FIELDS_SIZE > end)
                    return false;

                obj.colorR = ReadFloat(ptr);
                obj.colorG = ReadFloat(ptr + 4);
                obj.colorB = ReadFloat(ptr + 8);
                obj.intensity = ReadFloat(ptr + 12);
                ptr += V5_LIGHT_FIELDS_SIZE;

                if (obj.type == GameObjectType::SPOT_LIGHT)
                {
                    if (ptr + V5_SPOT_FIELDS_SIZE > end)
                        return false;

                    obj.innerCone = ReadFloat(ptr);
                    obj.outerCone = ReadFloat(ptr + 4);
                    ptr += V5_SPOT_FIELDS_SIZE;
                }
            }

            scene.gameObjects.push_back(obj);
        }

        return true;
    }

    void MapbinLoader::ComputeNormalsForGroup(MapbinGroup& group)
    {
        size_t vertexCount = static_cast<size_t>(group.vertexCount);

        std::vector<float> normals(vertexCount * 3, 0.0f);

        for (size_t i = 0; i + 2 < group.indices.size(); i += 3)
        {
            uint32_t i0 = group.indices[i];
            uint32_t i1 = group.indices[i + 1];
            uint32_t i2 = group.indices[i + 2];

            if (i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount)
                continue;

            size_t o0 = static_cast<size_t>(i0) * 8;
            size_t o1 = static_cast<size_t>(i1) * 8;
            size_t o2 = static_cast<size_t>(i2) * 8;

            float e1x = group.vertexData[o1 + 0] - group.vertexData[o0 + 0];
            float e1y = group.vertexData[o1 + 1] - group.vertexData[o0 + 1];
            float e1z = group.vertexData[o1 + 2] - group.vertexData[o0 + 2];

            float e2x = group.vertexData[o2 + 0] - group.vertexData[o0 + 0];
            float e2y = group.vertexData[o2 + 1] - group.vertexData[o0 + 1];
            float e2z = group.vertexData[o2 + 2] - group.vertexData[o0 + 2];

            float nx = e1y * e2z - e1z * e2y;
            float ny = e1z * e2x - e1x * e2z;
            float nz = e1x * e2y - e1y * e2x;

            normals[i0 * 3 + 0] += nx;
            normals[i0 * 3 + 1] += ny;
            normals[i0 * 3 + 2] += nz;

            normals[i1 * 3 + 0] += nx;
            normals[i1 * 3 + 1] += ny;
            normals[i1 * 3 + 2] += nz;

            normals[i2 * 3 + 0] += nx;
            normals[i2 * 3 + 1] += ny;
            normals[i2 * 3 + 2] += nz;
        }

        for (size_t v = 0; v < vertexCount; ++v)
        {
            float nx = normals[v * 3 + 0];
            float ny = normals[v * 3 + 1];
            float nz = normals[v * 3 + 2];

            float len = std::sqrt(nx * nx + ny * ny + nz * nz);

            if (len > 1e-6f)
            {
                nx /= len;
                ny /= len;
                nz /= len;
            }
            else
            {
                nx = 0.0f;
                ny = 1.0f;
                nz = 0.0f;
            }

            size_t offset = v * 8;
            group.vertexData[offset + 3] = nx;
            group.vertexData[offset + 4] = ny;
            group.vertexData[offset + 5] = nz;
        }
    }
}
