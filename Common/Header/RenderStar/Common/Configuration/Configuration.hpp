#pragma once

#include "RenderStar/Common/Configuration/IConfiguration.hpp"
#include <pugixml.hpp>
#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Common::Configuration
{
    class Configuration final : public IConfiguration
    {

    public:

        Configuration(std::string  configNamespace, std::string  classScope, std::shared_ptr<pugi::xml_document> sharedDocument, std::filesystem::path  sourceFilePath);

        Configuration(std::string  configNamespace, std::string  classScope, std::shared_ptr<pugi::xml_document> sharedDocument);

        [[nodiscard]]
        std::optional<std::string> GetString(const std::string& path) const override;

        [[nodiscard]]
        std::optional<int32_t> GetInteger(const std::string& path) const override;

        [[nodiscard]]
        std::optional<float> GetFloat(const std::string& path) const override;

        [[nodiscard]]
        std::optional<bool> GetBoolean(const std::string& path) const override;

        [[nodiscard]]
        std::vector<std::string> GetStringList(const std::string& path) const override;

        void SetString(const std::string& path, const std::string& value) override;
        void SetInteger(const std::string& path, int32_t value) override;
        void SetFloat(const std::string& path, float value) override;
        void SetBoolean(const std::string& path, bool value) override;

        void Save() override;
        void Reload() override;

        [[nodiscard]]
        const std::string& GetNamespace() const;

        [[nodiscard]]
        const std::string& GetClassScope() const;

    private:

        void InitializeScopedElement();

        [[nodiscard]]
        pugi::xml_node NavigateToNode(const std::string& path) const;

        pugi::xml_node NavigateOrCreateNode(const std::string& path) const;

        std::shared_ptr<spdlog::logger> logger;

        std::string configNamespace;
        std::string classScope;

        std::shared_ptr<pugi::xml_document> document;
        std::filesystem::path filePath;

        pugi::xml_node scopedElement;
    };
}
