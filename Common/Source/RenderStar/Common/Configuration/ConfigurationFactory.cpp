#include "RenderStar/Common/Configuration/ConfigurationFactory.hpp"

namespace RenderStar::Common::Configuration
{
    std::unordered_map<std::string, std::shared_ptr<IConfiguration>>& ConfigurationFactory::GetCache()
    {
        static std::unordered_map<std::string, std::shared_ptr<IConfiguration>> cache;
        return cache;
    }

    std::unordered_map<std::string, std::shared_ptr<pugi::xml_document>>& ConfigurationFactory::GetDocumentCache()
    {
        static std::unordered_map<std::string, std::shared_ptr<pugi::xml_document>> documentCache;
        return documentCache;
    }

    std::filesystem::path& ConfigurationFactory::GetResourceBasePath()
    {
        static std::filesystem::path basePath = std::filesystem::current_path();
        return basePath;
    }

    void ConfigurationFactory::SetResourceBasePath(const std::filesystem::path& path)
    {
        GetResourceBasePath() = path;
    }

    std::filesystem::path ConfigurationFactory::GetBasePath()
    {
        return GetResourceBasePath();
    }

    void ConfigurationFactory::ClearCache()
    {
        GetCache().clear();
        GetDocumentCache().clear();
    }

    std::shared_ptr<pugi::xml_document> ConfigurationFactory::GetOrLoadDocument(
        const std::string& documentKey,
        const std::filesystem::path& filePath)
    {
        auto& docCache = GetDocumentCache();
        auto it = docCache.find(documentKey);

        if (it != docCache.end())
            return it->second;

        auto doc = std::make_shared<pugi::xml_document>();
        pugi::xml_parse_result result = doc->load_file(filePath.c_str());

        if (!result)
        {
            spdlog::error("Failed to parse configuration file {}: {}", filePath.string(), result.description());
            return nullptr;
        }

        docCache[documentKey] = doc;
        return doc;
    }
}
