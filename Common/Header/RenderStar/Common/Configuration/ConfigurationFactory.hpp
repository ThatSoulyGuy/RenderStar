#pragma once

#include "RenderStar/Common/Configuration/IConfiguration.hpp"
#include "RenderStar/Common/Configuration/Configuration.hpp"
#include "RenderStar/Common/Configuration/EmptyConfiguration.hpp"
#include <memory>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <pugixml.hpp>
#include <spdlog/spdlog.h>

#ifdef _MSC_VER
#include <typeinfo>
#else
#include <cxxabi.h>
#endif

namespace RenderStar::Common::Configuration
{
    namespace Detail
    {
        template<typename T>
        std::string GetTypeName()
        {
#ifdef _MSC_VER
            std::string name = typeid(T).name();

            if (name.starts_with("class "))
                name = name.substr(6);
            else if (name.starts_with("struct "))
                name = name.substr(7);

            size_t pos = name.rfind("::");
            if (pos != std::string::npos)
                name = name.substr(pos + 2);

            return name;
#else
            int status = 0;
            char* demangled = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);

            if (status == 0 && demangled != nullptr)
            {
                std::string name(demangled);
                free(demangled);

                size_t pos = name.rfind("::");
                if (pos != std::string::npos)
                    name = name.substr(pos + 2);

                return name;
            }

            return typeid(T).name();
#endif
        }
    }

    class ConfigurationFactory
    {
    public:

        template<typename T>
        static std::shared_ptr<IConfiguration> For(const std::string& namespaceName)
        {
            return ForInternal<T>(namespaceName, "engine_settings.xml");
        }

        template<typename T>
        static std::shared_ptr<IConfiguration> For(const std::string& namespaceName, const std::string& fileName)
        {
            return ForInternal<T>(namespaceName, fileName);
        }

        template<typename T>
        static std::shared_ptr<IConfiguration> ForPath(const std::string& namespaceName, const std::filesystem::path& filePath)
        {
            std::string classScope = Detail::GetTypeName<T>();
            std::string cacheKey = namespaceName + ":" + classScope + ":" + filePath.string();

            auto& cache = GetCache();
            auto it = cache.find(cacheKey);

            if (it != cache.end())
                return it->second;

            if (!std::filesystem::exists(filePath))
            {
                spdlog::error("Configuration file not found: {}", filePath.string());
                auto empty = std::make_shared<EmptyConfiguration>();
                cache[cacheKey] = empty;
                return empty;
            }

            auto doc = GetOrLoadDocument(filePath.string(), filePath);

            if (!doc)
            {
                auto empty = std::make_shared<EmptyConfiguration>();
                cache[cacheKey] = empty;
                return empty;
            }

            spdlog::debug("Loading configuration from {} for {}", filePath.string(), classScope);

            auto config = std::make_shared<Configuration>(namespaceName, classScope, doc, filePath);
            cache[cacheKey] = config;

            return config;
        }

        static void SetResourceBasePath(const std::filesystem::path& path);

        static std::filesystem::path GetBasePath();

        static void ClearCache();

    private:

        template<typename T>
        static std::shared_ptr<IConfiguration> ForInternal(const std::string& namespaceName, const std::string& fileName)
        {
            std::string classScope = Detail::GetTypeName<T>();
            std::string cacheKey = namespaceName + ":" + classScope + ":" + fileName;

            auto& cache = GetCache();
            auto it = cache.find(cacheKey);

            if (it != cache.end())
                return it->second;

            std::string cleanNamespace = namespaceName;
            cleanNamespace.erase(std::remove(cleanNamespace.begin(), cleanNamespace.end(), '_'), cleanNamespace.end());

            std::filesystem::path resourcePath = GetResourceBasePath() / "assets" / cleanNamespace / "configuration" / fileName;
            spdlog::info("Loading configuration from: {}", resourcePath.string());

            if (!std::filesystem::exists(resourcePath))
            {
                spdlog::error("Configuration file not found: {}", resourcePath.string());
                auto empty = std::make_shared<EmptyConfiguration>();
                cache[cacheKey] = empty;
                return empty;
            }

            auto doc = GetOrLoadDocument(namespaceName + ":" + fileName, resourcePath);

            if (!doc)
            {
                auto empty = std::make_shared<EmptyConfiguration>();
                cache[cacheKey] = empty;
                return empty;
            }

            spdlog::debug("Loading configuration from {} for {}", resourcePath.string(), classScope);

            auto config = std::make_shared<Configuration>(namespaceName, classScope, doc, resourcePath);
            cache[cacheKey] = config;

            return config;
        }

        static std::unordered_map<std::string, std::shared_ptr<IConfiguration>>& GetCache();
        static std::unordered_map<std::string, std::shared_ptr<pugi::xml_document>>& GetDocumentCache();
        static std::filesystem::path& GetResourceBasePath();

        static std::shared_ptr<pugi::xml_document> GetOrLoadDocument(
            const std::string& documentKey,
            const std::filesystem::path& filePath);
    };
}
