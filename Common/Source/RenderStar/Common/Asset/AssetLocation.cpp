#include "RenderStar/Common/Asset/AssetLocation.hpp"

namespace RenderStar::Common::Asset
{
    AssetLocation::AssetLocation(std::string_view namespaceId, std::string_view path)
        : namespaceId(namespaceId)
        , path(path)
    {
    }

    AssetLocation AssetLocation::Parse(std::string_view identifier)
    {
        auto separatorPosition = identifier.find(NAMESPACE_SEPARATOR);

        if (separatorPosition == std::string_view::npos)
            return AssetLocation(DEFAULT_NAMESPACE, identifier);

        return AssetLocation(
            identifier.substr(0, separatorPosition),
            identifier.substr(separatorPosition + 1)
        );
    }

    AssetLocation AssetLocation::Of(std::string_view namespaceId, std::string_view path)
    {
        return AssetLocation(namespaceId, path);
    }

    const std::string& AssetLocation::GetNamespace() const
    {
        return namespaceId;
    }

    const std::string& AssetLocation::GetPath() const
    {
        return path;
    }

    std::string AssetLocation::ToString() const
    {
        return namespaceId + NAMESPACE_SEPARATOR + path;
    }

    std::filesystem::path AssetLocation::ToFilesystemPath(const std::filesystem::path& basePath) const
    {
        return basePath / "assets" / namespaceId / path;
    }

    bool AssetLocation::IsValid() const
    {
        return !namespaceId.empty() && !path.empty();
    }
}
