#include "RenderStar/Common/Asset/FilesystemAssetProvider.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace RenderStar::Common::Asset
{
    FilesystemAssetProvider::FilesystemAssetProvider(const std::string_view namespaceId, std::filesystem::path basePath) : namespaceId(namespaceId), basePath(std::move(basePath)) { }

    std::string FilesystemAssetProvider::GetNamespace() const
    {
        return namespaceId;
    }

    bool FilesystemAssetProvider::Exists(const AssetLocation& location) const
    {
        if (location.GetNamespace() != namespaceId)
            return false;

        return std::filesystem::exists(GetFullPath(location));
    }

    std::vector<uint8_t> FilesystemAssetProvider::LoadBinary(const AssetLocation& location)
    {
        const auto fullPath = GetFullPath(location);
        std::ifstream file(fullPath, std::ios::binary | std::ios::ate);

        if (!file.is_open())
            throw std::runtime_error("Failed to open asset: " + location.ToString());

        const auto fileSize = file.tellg();
        file.seekg(0);

        std::vector<uint8_t> data(fileSize);
        file.read(reinterpret_cast<char*>(data.data()), fileSize);

        return data;
    }

    std::string FilesystemAssetProvider::LoadText(const AssetLocation& location)
    {
        auto fullPath = GetFullPath(location);
        std::ifstream file(fullPath);

        if (!file.is_open())
            throw std::runtime_error("Failed to open asset: " + location.ToString());

        std::stringstream buffer;
        buffer << file.rdbuf();

        return buffer.str();
    }

    std::vector<AssetLocation> FilesystemAssetProvider::List(std::string_view pathPrefix) const
    {
        std::vector<AssetLocation> results;

        const auto searchPath = basePath / pathPrefix;

        if (!std::filesystem::exists(searchPath))
            return results;

        CollectAssets(searchPath, pathPrefix, results);

        return results;
    }

    std::filesystem::path FilesystemAssetProvider::GetFullPath(const AssetLocation& location) const
    {
        return basePath / location.GetPath();
    }

    void FilesystemAssetProvider::CollectAssets(const std::filesystem::path& directory, const std::string_view pathPrefix, std::vector<AssetLocation>& results) const
    {
        if (!std::filesystem::is_directory(directory))
            return;

        for (const auto& entry : std::filesystem::directory_iterator(directory))
        {
            if (entry.is_regular_file())
            {
                auto relativePath = std::filesystem::relative(entry.path(), basePath);
                results.emplace_back(AssetLocation::Of(namespaceId, relativePath.generic_string()));
            }
            else if (entry.is_directory())
            {
                CollectAssets(entry.path(), pathPrefix, results);
            }
        }
    }
}
