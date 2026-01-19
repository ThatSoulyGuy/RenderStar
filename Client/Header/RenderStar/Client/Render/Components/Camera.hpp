#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace RenderStar::Client::Render::Components
{
    enum class ProjectionType
    {
        PERSPECTIVE,
        ORTHOGRAPHIC
    };

    struct Camera
    {
        ProjectionType projectionType{ ProjectionType::PERSPECTIVE };
        float fieldOfView{ 60.0f };
        float nearPlane{ 0.1f };
        float farPlane{ 1000.0f };
        float aspectRatio{ 16.0f / 9.0f };
        float orthographicSize{ 10.0f };

        glm::mat4 viewMatrix{ 1.0f };
        glm::mat4 projectionMatrix{ 1.0f };

        static Camera CreatePerspective(float fov, float aspect, float nearClip, float farClip);

        static Camera CreateOrthographic(float size, float aspect, float nearClip, float farClip);

        glm::mat4 GetViewMatrix() const;

        glm::mat4 GetProjectionMatrix() const;

        glm::mat4 GetViewProjectionMatrix() const;
    };
}
