#include "RenderStar/Common/Asset/BinaryAssetLoader.hpp"
#include "RenderStar/Common/Asset/BinaryAsset.hpp"

namespace RenderStar::Common::Asset
{
    std::shared_ptr<IBinaryAsset> BinaryAssetLoader::Load(const AssetLocation& location, IAssetProvider& provider)
    {
        auto data = provider.LoadBinary(location);
        return std::make_shared<BinaryAsset>(location, std::move(data));
    }

    std::vector<std::string> BinaryAssetLoader::GetSupportedExtensions() const
    {
        return { ".spv", ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".dds", ".ktx", ".bin" };
    }
}
