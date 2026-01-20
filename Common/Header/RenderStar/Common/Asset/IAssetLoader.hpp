#pragma once

#include "RenderStar/Common/Asset/AssetLocation.hpp"
#include "RenderStar/Common/Asset/IAssetProvider.hpp"
#include <memory>
#include <vector>
#include <string>

namespace RenderStar::Common::Asset
{
    template<typename T>
    class IAssetLoader
    {
    public:
        virtual ~IAssetLoader() = default;

        virtual std::shared_ptr<T> Load(const AssetLocation& location, IAssetProvider& provider) = 0;
        virtual std::vector<std::string> GetSupportedExtensions() const = 0;
    };
}
