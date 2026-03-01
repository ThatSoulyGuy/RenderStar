#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace RenderStar::Common::Scene
{
    enum class TextureSlotType : uint32_t
    {
        BASE_COLOR = 0,
        NORMAL = 1,
        ROUGHNESS = 2,
        METALLIC = 3,
        SPECULAR = 4,
        DETAIL_ALBEDO = 6,
        DETAIL_NORMAL = 7,
        EMISSION = 8,
        AO = 9
    };

    enum class GameObjectType : uint32_t
    {
        PLAYER_START = 0,
        POINT_LIGHT = 1,
        SPOT_LIGHT = 2,
        SUN_LIGHT = 3
    };

    struct MapbinGameObject
    {
        GameObjectType type = GameObjectType::PLAYER_START;
        float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
        float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;
        float colorR = 1.0f, colorG = 1.0f, colorB = 1.0f;
        float intensity = 1.0f;
        float innerCone = 0.0f, outerCone = 0.0f;
    };

    struct MapbinTextureSlot
    {
        TextureSlotType slotType = TextureSlotType::BASE_COLOR;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t wrapS = 0;
        uint32_t wrapT = 0;
        uint32_t minFilter = 0;
        uint32_t magFilter = 0;
        std::vector<uint8_t> pixelData;
    };

    struct MapbinMaterial
    {
        int32_t materialId = 0;
        float normalStrength = 1.0f;
        float roughness = 0.5f;
        float metallic = 0.0f;
        float specularStrength = 0.5f;
        float detailScale = 1.0f;
        float emissionStrength = 1.0f;
        float aoStrength = 1.0f;
        std::vector<MapbinTextureSlot> textureSlots;
    };

    struct MapbinGroup
    {
        std::vector<float> vertexData;
        std::vector<uint32_t> indices;
        int32_t vertexCount = 0;
        int32_t materialId = 0;
    };

    struct MapbinScene
    {
        std::vector<MapbinMaterial> materials;
        std::vector<MapbinGroup> groups;
        std::vector<MapbinGameObject> gameObjects;
    };

    class MapbinLoader
    {
    public:

        static std::optional<MapbinScene> Load(std::span<const uint8_t> data);

    private:

        static std::optional<MapbinScene> LoadV2(const uint8_t* ptr, const uint8_t* end,
            uint32_t textureCount, uint32_t groupCount);

        static std::optional<MapbinScene> LoadV3(const uint8_t* ptr, const uint8_t* end,
            uint32_t materialCount, uint32_t groupCount);

        static std::optional<MapbinScene> LoadV4(const uint8_t* ptr, const uint8_t* end,
            uint32_t materialCount, uint32_t groupCount);

        static std::optional<MapbinScene> LoadV5(const uint8_t* ptr, const uint8_t* end,
            uint32_t materialCount, uint32_t groupCount);

        static bool ParseMaterialsAndGroups(const uint8_t*& ptr, const uint8_t* end,
            uint32_t materialCount, uint32_t groupCount, MapbinScene& scene);

        static bool ParseGameObjectsV4(const uint8_t*& ptr, const uint8_t* end, MapbinScene& scene);
        static bool ParseGameObjectsV5(const uint8_t*& ptr, const uint8_t* end, MapbinScene& scene);

        static void ComputeNormalsForGroup(MapbinGroup& group);

        static constexpr uint32_t MAGIC = 0x4D415042;
        static constexpr uint32_t VERSION_2 = 2;
        static constexpr uint32_t VERSION_3 = 3;
        static constexpr uint32_t VERSION_4 = 4;
        static constexpr uint32_t VERSION_5 = 5;
        static constexpr size_t HEADER_SIZE = 16;
        static constexpr size_t V2_TEXTURE_HEADER_SIZE = 32;
        static constexpr size_t V3_MATERIAL_SCALAR_SIZE = 32;
        static constexpr size_t V3_SLOT_HEADER_SIZE = 32;
        static constexpr size_t VERTEX_SIZE = 32;
        static constexpr size_t GROUP_HEADER_SIZE = 12;
        static constexpr size_t V4_GAME_OBJECT_SIZE = 16;
        static constexpr size_t V5_GAME_OBJECT_COMMON_SIZE = 28;
        static constexpr size_t V5_LIGHT_FIELDS_SIZE = 16;
        static constexpr size_t V5_SPOT_FIELDS_SIZE = 8;
    };
}
