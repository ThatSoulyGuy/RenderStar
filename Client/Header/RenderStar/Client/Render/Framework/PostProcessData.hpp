#pragma once

#include <cstddef>
#include <glm/glm.hpp>

namespace RenderStar::Client::Render::Framework
{
    struct PostProcessData
    {
        float exposureBias = 1.0f;
        float exposureMin = 0.1f;
        float exposureMax = 10.0f;
        float bloomIntensity = 0.04f;

        float contrast = 1.05f;
        float saturation = 1.1f;
        float vignetteStrength = 0.15f;
        float temperature = 0.0f;

        glm::vec4 fogColor{0.7f, 0.8f, 0.9f, 0.0f};
        glm::vec4 colorFilter{1.0f, 1.0f, 1.0f, 0.0f};

        static constexpr size_t Size() { return sizeof(PostProcessData); }

        static PostProcessData Defaults()
        {
            return PostProcessData{};
        }
    };
}
