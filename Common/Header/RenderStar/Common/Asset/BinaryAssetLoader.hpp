#pragma once

#include "RenderStar/Common/Asset/IAssetLoader.hpp"
#include "RenderStar/Common/Asset/IBinaryAsset.hpp"

namespace RenderStar::Common::Asset
{
    class BinaryAssetLoader : public IAssetLoader<IBinaryAsset>
    {
    public:
        std::shared_ptr<IBinaryAsset> Load(const AssetLocation& location, IAssetProvider& provider) override;
        std::vector<std::string> GetSupportedExtensions() const override;
    };
}
