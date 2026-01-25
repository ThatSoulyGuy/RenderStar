#include "RenderStar/Common/Configuration/ConfigurationModule.hpp"

namespace RenderStar::Common::Configuration
{
    ConfigurationModule::ConfigurationModule(std::filesystem::path resourceBasePath)
        : resourceBasePath(std::move(resourceBasePath))
    {
    }

    const std::filesystem::path& ConfigurationModule::GetBasePath() const
    {
        return resourceBasePath;
    }

    void ConfigurationModule::ClearCache()
    {
        configCache.clear();
        documentCache.clear();
    }

    void ConfigurationModule::OnInitialize(Module::ModuleContext& context)
    {
        logger->info("ConfigurationModule initialized with base path: {}", resourceBasePath.string());
    }

    std::shared_ptr<pugi::xml_document> ConfigurationModule::GetOrLoadDocument(const std::string& documentKey, const std::filesystem::path& filePath)
    {
        if (const auto iterator = documentCache.find(documentKey); iterator != documentCache.end())
            return iterator->second;

        auto document = std::make_shared<pugi::xml_document>();

        if (const pugi::xml_parse_result result = document->load_file(filePath.c_str()); !result)
        {
            logger->error("Failed to parse configuration file {}: {}", filePath.string(), result.description());
            return nullptr;
        }

        documentCache[documentKey] = document;

        return document;
    }
}
