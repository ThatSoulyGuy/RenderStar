#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <functional>

namespace RenderStar::Common::Asset
{
    class AssetLocation
    {
    public:
        static constexpr std::string_view DEFAULT_NAMESPACE = "renderstar";
        static constexpr char NAMESPACE_SEPARATOR = ':';

        AssetLocation() = default;
        AssetLocation(std::string_view namespaceId, std::string_view path);

        static AssetLocation Parse(std::string_view identifier);
        static AssetLocation Of(std::string_view namespaceId, std::string_view path);

        const std::string& GetNamespace() const;
        const std::string& GetPath() const;
        std::string ToString() const;
        std::filesystem::path ToFilesystemPath(const std::filesystem::path& basePath) const;

        bool IsValid() const;

        auto operator<=>(const AssetLocation&) const = default;
        bool operator==(const AssetLocation&) const = default;

    private:
        std::string namespaceId;
        std::string path;
    };
}

template<>
struct std::hash<RenderStar::Common::Asset::AssetLocation>
{
    std::size_t operator()(const RenderStar::Common::Asset::AssetLocation& location) const noexcept
    {
        std::size_t h1 = std::hash<std::string>{}(location.GetNamespace());
        std::size_t h2 = std::hash<std::string>{}(location.GetPath());
        return h1 ^ (h2 << 1);
    }
};
