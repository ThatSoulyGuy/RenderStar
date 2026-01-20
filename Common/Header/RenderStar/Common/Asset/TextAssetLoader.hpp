#pragma once

#include "RenderStar/Common/Asset/IAssetLoader.hpp"
#include "RenderStar/Common/Asset/ITextAsset.hpp"

namespace RenderStar::Common::Asset
{
    class TextAssetLoader : public IAssetLoader<ITextAsset>
    {
    public:
        std::shared_ptr<ITextAsset> Load(const AssetLocation& location, IAssetProvider& provider) override;
        std::vector<std::string> GetSupportedExtensions() const override;
    };
}
