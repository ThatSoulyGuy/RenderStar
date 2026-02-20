#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace RenderStar::Common::Scene
{
    struct MapbinGroup
    {
        std::vector<float> vertexData;
        std::vector<uint32_t> indices;
        int32_t vertexCount = 0;
    };

    struct MapbinScene
    {
        std::vector<MapbinGroup> groups;
    };

    class MapbinLoader
    {
    public:

        static std::optional<MapbinScene> Load(std::span<const uint8_t> data);

    private:

        static constexpr uint32_t MAGIC = 0x4D415042;
        static constexpr uint32_t VERSION = 2;
        static constexpr size_t HEADER_SIZE = 16;
        static constexpr size_t TEXTURE_HEADER_SIZE = 32;
        static constexpr size_t VERTEX_SIZE = 32;
        static constexpr size_t GROUP_HEADER_SIZE = 12;
    };
}
