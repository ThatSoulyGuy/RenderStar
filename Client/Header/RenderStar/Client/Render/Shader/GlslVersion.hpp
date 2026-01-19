#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <optional>

namespace RenderStar::Client::Render::Shader
{
    enum class GlslVersion
    {
        GLSL_330,
        GLSL_410,
        GLSL_450,
        GLSL_460
    };

    inline int32_t GetVersionNumber(GlslVersion version)
    {
        switch (version)
        {
            case GlslVersion::GLSL_330: return 330;
            case GlslVersion::GLSL_410: return 410;
            case GlslVersion::GLSL_450: return 450;
            case GlslVersion::GLSL_460: return 460;
        }
        return 330;
    }

    inline std::string_view GetVersionString(GlslVersion version)
    {
        switch (version)
        {
            case GlslVersion::GLSL_330: return "330 core";
            case GlslVersion::GLSL_410: return "410 core";
            case GlslVersion::GLSL_450: return "450";
            case GlslVersion::GLSL_460: return "460 core";
        }
        return "330 core";
    }

    inline bool IsOpenGLCompatible(GlslVersion version)
    {
        switch (version)
        {
            case GlslVersion::GLSL_330: return true;
            case GlslVersion::GLSL_410: return true;
            case GlslVersion::GLSL_450: return false;
            case GlslVersion::GLSL_460: return true;
        }
        return false;
    }

    inline std::string GetVersionDirective(GlslVersion version)
    {
        return "#version " + std::string(GetVersionString(version));
    }

    inline std::optional<GlslVersion> ParseVersionFromSource(const std::string& source)
    {
        if (source.empty())
            return std::nullopt;

        size_t pos = 0;
        while (pos < source.size())
        {
            size_t lineEnd = source.find('\n', pos);
            if (lineEnd == std::string::npos)
                lineEnd = source.size();

            std::string_view line(source.data() + pos, lineEnd - pos);

            size_t trimStart = line.find_first_not_of(" \t\r");
            if (trimStart != std::string_view::npos)
                line = line.substr(trimStart);

            if (line.starts_with("#version"))
            {
                if (line.find("450") != std::string_view::npos)
                    return GlslVersion::GLSL_450;
                if (line.find("460") != std::string_view::npos)
                    return GlslVersion::GLSL_460;
                if (line.find("410") != std::string_view::npos)
                    return GlslVersion::GLSL_410;
                if (line.find("330") != std::string_view::npos)
                    return GlslVersion::GLSL_330;
            }

            pos = lineEnd + 1;
        }

        return std::nullopt;
    }
}
