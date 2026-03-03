#pragma once

#include "RenderStar/Client/Render/Framework/PostProcessData.hpp"
#include <cstdint>
#include <glm/glm.hpp>

namespace RenderStar::Client::Render::Components
{
    enum PostProcessOverride : uint32_t
    {
        PP_NONE            = 0,
        PP_EXPOSURE_BIAS   = 1 << 0,
        PP_BLOOM_INTENSITY = 1 << 1,
        PP_CONTRAST        = 1 << 2,
        PP_SATURATION      = 1 << 3,
        PP_VIGNETTE        = 1 << 4,
        PP_TEMPERATURE     = 1 << 5,
        PP_FOG             = 1 << 6,
        PP_COLOR_FILTER    = 1 << 7,
        PP_ALL             = 0xFF
    };

    struct AdaptiveVolume
    {
        glm::vec3 halfExtents = glm::vec3(5.0f);
        int32_t priority = 0;
        float blendDistance = 1.0f;
        uint32_t overrideMask = PP_ALL;
        Framework::PostProcessData settings = Framework::PostProcessData::Defaults();
    };
}
