#pragma once

#include "RenderStar/Common/Configuration/IConfiguration.hpp"
#include <pugixml.hpp>
#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Common::Configuration
{
    class Configuration : public IConfiguration
    {
    public:

        Configuration(
            const std::string& configNamespace,
            const std::string& classScope,
            std::shared_ptr<pugi::xml_document> sharedDocument,
            const std::filesystem::path& sourceFilePath);

        Configuration(
            const std::string& configNamespace,
            const std::string& classScope,
            std::shared_ptr<pugi::xml_document> sharedDocument);

        std::optional<std::string> GetString(const std::string& path) const override;
        std::optional<int32_t> GetInteger(const std::string& path) const override;
        std::optional<float> GetFloat(const std::string& path) const override;
        std::optional<bool> GetBoolean(const std::string& path) const override;
        std::vector<std::string> GetStringList(const std::string& path) const override;

        void SetString(const std::string& path, const std::string& value) override;
        void SetInteger(const std::string& path, int32_t value) override;
        void SetFloat(const std::string& path, float value) override;
        void SetBoolean(const std::string& path, bool value) override;

        void Save() override;
        void Reload() override;

        const std::string& GetNamespace() const;
        const std::string& GetClassScope() const;

    private:

        void InitializeScopedElement();
        pugi::xml_node NavigateToNode(const std::string& path) const;
        pugi::xml_node NavigateOrCreateNode(const std::string& path);

        std::shared_ptr<spdlog::logger> logger;
        std::string configNamespace;
        std::string classScope;
        std::shared_ptr<pugi::xml_document> document;
        std::filesystem::path filePath;
        pugi::xml_node scopedElement;
    };
}
