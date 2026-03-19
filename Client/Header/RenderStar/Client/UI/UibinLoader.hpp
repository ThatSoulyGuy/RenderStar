#pragma once

#include "RenderStar/Client/UI/UibinScene.hpp"
#include <cstdint>
#include <optional>
#include <span>

namespace RenderStar::Common::Asset
{
    class AssetModule;
    class AssetLocation;
}

namespace RenderStar::Client::UI
{
    class UibinLoader
    {
    public:

        static std::optional<UibinScene> Load(const Common::Asset::AssetLocation& location, Common::Asset::AssetModule& assetModule);
        static std::optional<UibinScene> Parse(std::span<const uint8_t> data);

    private:

        static constexpr uint32_t MAGIC       = 0x5549424E;
        static constexpr uint32_t VERSION     = 1;
        static constexpr size_t   HEADER_SIZE = 16;
    };
}
