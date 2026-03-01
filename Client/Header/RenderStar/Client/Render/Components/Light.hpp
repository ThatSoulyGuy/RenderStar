#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace RenderStar::Client::Render::Components
{
    enum class LightType : uint32_t
    {
        DIRECTIONAL,
        POINT,
        SPOT
    };

    struct Light
    {
        LightType type = LightType::DIRECTIONAL;
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;
        glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
        float range = 10.0f;
        float spotAngle = 45.0f;
        float spotSoftness = 0.1f;
        bool castShadows = false;

        static Light Directional(glm::vec3 direction, glm::vec3 color, float intensity);
        static Light Point(glm::vec3 color, float intensity, float range);
        static Light Spot(glm::vec3 direction, glm::vec3 color, float intensity, float range, float angle);
    };
}
