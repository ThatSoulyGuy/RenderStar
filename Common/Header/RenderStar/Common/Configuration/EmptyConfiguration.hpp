#pragma once

#include "RenderStar/Common/Configuration/IConfiguration.hpp"

namespace RenderStar::Common::Configuration
{
    class EmptyConfiguration : public IConfiguration
    {
    public:

        std::optional<std::string> GetString(const std::string& path) const override
        {
            (void)path;
            return std::nullopt;
        }

        std::optional<int32_t> GetInteger(const std::string& path) const override
        {
            (void)path;
            return std::nullopt;
        }

        std::optional<float> GetFloat(const std::string& path) const override
        {
            (void)path;
            return std::nullopt;
        }

        std::optional<bool> GetBoolean(const std::string& path) const override
        {
            (void)path;
            return std::nullopt;
        }

        std::vector<std::string> GetStringList(const std::string& path) const override
        {
            (void)path;
            return {};
        }

        void SetString(const std::string& path, const std::string& value) override
        {
            (void)path;
            (void)value;
        }

        void SetInteger(const std::string& path, int32_t value) override
        {
            (void)path;
            (void)value;
        }

        void SetFloat(const std::string& path, float value) override
        {
            (void)path;
            (void)value;
        }

        void SetBoolean(const std::string& path, bool value) override
        {
            (void)path;
            (void)value;
        }

        void Save() override
        {
        }

        void Reload() override
        {
        }
    };
}
