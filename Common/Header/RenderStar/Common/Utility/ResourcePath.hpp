#pragma once

#include <string>
#include <filesystem>

namespace RenderStar::Common::Utility
{
    struct ResourcePath
    {
        std::string domain;
        std::string localPath;

        ResourcePath() = default;

        ResourcePath(std::string domainValue, std::string pathValue) : domain(std::move(domainValue)), localPath(std::move(pathValue)) { }

        [[nodiscard]]
        std::string FullPath() const
        {
            return "/assets/" + domain + "/" + localPath;
        }

        [[nodiscard]]
        std::string RelativePath() const
        {
            return "assets/" + domain + "/" + localPath;
        }

        [[nodiscard]]
        std::filesystem::path ToFilesystemPath(const std::filesystem::path& basePath) const
        {
            return basePath / "assets" / domain / localPath;
        }

        bool operator==(const ResourcePath& other) const
        {
            return domain == other.domain && localPath == other.localPath;
        }
    };
}
