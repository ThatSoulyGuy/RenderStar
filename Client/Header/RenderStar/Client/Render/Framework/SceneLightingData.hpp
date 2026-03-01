#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace RenderStar::Client::Render::Framework
{
    static constexpr int32_t MAX_POINT_LIGHTS = 16;
    static constexpr int32_t MAX_SPOT_LIGHTS = 8;

    struct PointLightGPU
    {
        glm::vec4 positionAndRange;
        glm::vec4 colorAndIntensity;
    };

    struct SpotLightGPU
    {
        glm::vec4 positionAndRange;
        glm::vec4 directionAndAngle;
        glm::vec4 colorAndIntensity;
        glm::vec4 spotParams{0.0f};
    };

    struct SceneLightingData
    {
        glm::vec4 cameraPosition{0.0f};
        glm::vec4 ambientColor{0.0f};
        glm::vec4 directionalDirection{0.0f};
        glm::vec4 directionalColor{0.0f};
        int32_t pointLightCount = 0;
        int32_t spotLightCount = 0;
        int32_t _pad0 = 0;
        int32_t _pad1 = 0;
        glm::mat4 directionalLightVP{1.0f};
        glm::vec4 shadowParams{0.0f};
        PointLightGPU pointLights[MAX_POINT_LIGHTS]{};
        SpotLightGPU spotLights[MAX_SPOT_LIGHTS]{};

        static constexpr size_t Size() { return sizeof(SceneLightingData); }
    };
}
