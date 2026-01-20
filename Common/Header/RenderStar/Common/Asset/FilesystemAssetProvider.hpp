#pragma once

#include "RenderStar/Common/Asset/IAssetProvider.hpp"
#include <filesystem>

namespace RenderStar::Common::Asset
{
    class FilesystemAssetProvider : public IAssetProvider
    {
    public:
        FilesystemAssetProvider(std::string_view namespaceId, const std::filesystem::path& basePath);

        std::string GetNamespace() const override;
        bool Exists(const AssetLocation& location) const override;
        std::vector<uint8_t> LoadBinary(const AssetLocation& location) override;
        std::string LoadText(const AssetLocation& location) override;
        std::vector<AssetLocation> List(std::string_view pathPrefix) const override;

    private:
        std::filesystem::path GetFullPath(const AssetLocation& location) const;
        void CollectAssets(const std::filesystem::path& directory, std::string_view pathPrefix, std::vector<AssetLocation>& results) const;

        std::string namespaceId;
        std::filesystem::path basePath;
    };
}
