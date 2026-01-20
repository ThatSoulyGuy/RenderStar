#pragma once

#include <glm/glm.hpp>

namespace RenderStar::Client::Render
{
    struct StandardUniforms
    {
        glm::mat4 model;
        glm::mat4 viewProjection;
        glm::vec4 colorTint;

        StandardUniforms()
            : model(1.0f)
            , viewProjection(1.0f)
            , colorTint(1.0f, 1.0f, 1.0f, 0.0f)
        {
        }

        StandardUniforms(const glm::mat4& model, const glm::mat4& viewProjection, const glm::vec4& colorTint)
            : model(model)
            , viewProjection(viewProjection)
            , colorTint(colorTint)
        {
        }

        static constexpr size_t Size()
        {
            return sizeof(StandardUniforms);
        }
    };
}
