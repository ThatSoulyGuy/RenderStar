#pragma once

#include "RenderStar/Common/Configuration/IConfiguration.hpp"
#include "RenderStar/Common/Configuration/Configuration.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Common/Utility/TypeName.hpp"
#include <memory>
#include <optional>
#include <string>
#include <filesystem>
#include <unordered_map>

namespace RenderStar::Common::Configuration
{
    class ConfigurationModule final : public Module::AbstractModule
    {
    public:

        explicit ConfigurationModule(std::filesystem::path resourceBasePath);

        template <typename T>
        std::optional<std::shared_ptr<IConfiguration>> For(const std::string& namespaceName)
        {
            return ForInternal<T>(namespaceName, "engine_settings.xml");
        }

        template <typename T>
        std::optional<std::shared_ptr<IConfiguration>> For(const std::string& namespaceName, const std::string& fileName)
        {
            return ForInternal<T>(namespaceName, fileName);
        }

        template <typename T>
        std::optional<std::shared_ptr<IConfiguration>> ForPath(const std::string& namespaceName, const std::filesystem::path& filePath)
        {
            std::string classScope = Utility::TypeName::Get<T>();
            const std::string cacheKey = namespaceName + ":" + classScope + ":" + filePath.string();

            if (const auto iterator = configCache.find(cacheKey); iterator != configCache.end())
                return iterator->second;

            if (!std::filesystem::exists(filePath))
            {
                logger->warn("Configuration file not found: {}", filePath.string());
                return std::nullopt;
            }

            auto document = GetOrLoadDocument(filePath.string(), filePath);

            if (!document)
                return std::nullopt;

            logger->debug("Loading configuration from {} for {}", filePath.string(), classScope);

            auto config = std::make_shared<Configuration>(namespaceName, classScope, document, filePath);
            configCache[cacheKey] = config;

            return config;
        }

        const std::filesystem::path& GetBasePath() const;

        void ClearCache();

    protected:

        void OnInitialize(Module::ModuleContext& context) override;

    private:

        template <typename T>
        std::optional<std::shared_ptr<IConfiguration>> ForInternal(const std::string& namespaceName, const std::string& fileName)
        {
            std::string classScope = Utility::TypeName::Get<T>();
            const std::string cacheKey = namespaceName + ":" + classScope + ":" + fileName;

            if (const auto iterator = configCache.find(cacheKey); iterator != configCache.end())
                return iterator->second;

            std::string cleanNamespace = namespaceName;
            std::erase(cleanNamespace, '_');

            std::filesystem::path resourcePath = resourceBasePath / "assets" / cleanNamespace / "configuration" / fileName;
            logger->debug("Looking for configuration at: {}", resourcePath.string());

            if (!std::filesystem::exists(resourcePath))
            {
                logger->warn("Configuration file not found: {}", resourcePath.string());
                return std::nullopt;
            }

            auto document = GetOrLoadDocument(namespaceName + ":" + fileName, resourcePath);

            if (!document)
                return std::nullopt;

            logger->info("Loaded configuration from {} for {}", resourcePath.string(), classScope);

            auto config = std::make_shared<Configuration>(namespaceName, classScope, document, resourcePath);
            configCache[cacheKey] = config;

            return config;
        }

        std::shared_ptr<pugi::xml_document> GetOrLoadDocument(const std::string& documentKey, const std::filesystem::path& filePath);

        std::filesystem::path resourceBasePath;
        std::unordered_map<std::string, std::shared_ptr<IConfiguration>> configCache;
        std::unordered_map<std::string, std::shared_ptr<pugi::xml_document>> documentCache;
    };
}
