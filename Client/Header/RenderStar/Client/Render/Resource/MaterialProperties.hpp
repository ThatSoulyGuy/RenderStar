#pragma once

#include <glm/glm.hpp>

namespace RenderStar::Client::Render
{
    struct MaterialProperties
    {
        glm::vec4 params{0.5f, 0.0f, 1.0f, 0.0f};
        glm::vec4 params2{1.0f, 0.5f, 0.0f, 0.0f};

        MaterialProperties() = default;

        MaterialProperties(float roughness, float metallic, float aoStrength, float emissionStrength,
                           float normalStrength = 1.0f, float specularStrength = 0.5f,
                           float detailScale = 0.0f)
            : params(roughness, metallic, aoStrength, emissionStrength)
            , params2(normalStrength, specularStrength, detailScale, 0.0f)
        {
        }

        static constexpr size_t Size()
        {
            return sizeof(MaterialProperties);
        }
    };
}
