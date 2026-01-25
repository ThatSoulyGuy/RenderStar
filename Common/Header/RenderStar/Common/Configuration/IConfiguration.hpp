#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace RenderStar::Common::Configuration
{
    class IConfiguration
    {
    public:

        virtual ~IConfiguration() = default;

        [[nodiscard]]
        virtual std::optional<std::string> GetString(const std::string& path) const = 0;

        [[nodiscard]]
        virtual std::optional<int32_t> GetInteger(const std::string& path) const = 0;

        [[nodiscard]]
        virtual std::optional<float> GetFloat(const std::string& path) const = 0;

        [[nodiscard]]
        virtual std::optional<bool> GetBoolean(const std::string& path) const = 0;

        [[nodiscard]]
        virtual std::vector<std::string> GetStringList(const std::string& path) const = 0;

        virtual void SetString(const std::string& path, const std::string& value) = 0;

        virtual void SetInteger(const std::string& path, int32_t value) = 0;

        virtual void SetFloat(const std::string& path, float value) = 0;

        virtual void SetBoolean(const std::string& path, bool value) = 0;

        virtual void Save() = 0;

        virtual void Reload() = 0;
    };
}
