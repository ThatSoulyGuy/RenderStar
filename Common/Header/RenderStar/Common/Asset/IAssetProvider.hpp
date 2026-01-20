#pragma once

#include "RenderStar/Common/Asset/AssetLocation.hpp"
#include <vector>
#include <string>
#include <cstdint>

namespace RenderStar::Common::Asset
{
    class IAssetProvider
    {
    public:
        virtual ~IAssetProvider() = default;

        virtual std::string GetNamespace() const = 0;
        virtual bool Exists(const AssetLocation& location) const = 0;
        virtual std::vector<uint8_t> LoadBinary(const AssetLocation& location) = 0;
        virtual std::string LoadText(const AssetLocation& location) = 0;
        virtual std::vector<AssetLocation> List(std::string_view pathPrefix) const = 0;
    };
}
