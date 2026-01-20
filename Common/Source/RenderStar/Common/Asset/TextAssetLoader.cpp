#include "RenderStar/Common/Asset/TextAssetLoader.hpp"
#include "RenderStar/Common/Asset/TextAsset.hpp"

namespace RenderStar::Common::Asset
{
    std::shared_ptr<ITextAsset> TextAssetLoader::Load(const AssetLocation& location, IAssetProvider& provider)
    {
        auto content = provider.LoadText(location);
        return std::make_shared<TextAsset>(location, std::move(content));
    }

    std::vector<std::string> TextAssetLoader::GetSupportedExtensions() const
    {
        return { ".txt", ".xml", ".json", ".glsl", ".vert", ".frag", ".comp", ".geom", ".tesc", ".tese" };
    }
}
