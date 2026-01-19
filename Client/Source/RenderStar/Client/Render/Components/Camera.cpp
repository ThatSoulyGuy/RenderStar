#include "RenderStar/Client/Render/Components/Camera.hpp"

namespace RenderStar::Client::Render::Components
{
    Camera Camera::CreatePerspective(float fov, float aspect, float nearClip, float farClip)
    {
        Camera camera;
        camera.projectionType = ProjectionType::PERSPECTIVE;
        camera.fieldOfView = fov;
        camera.aspectRatio = aspect;
        camera.nearPlane = nearClip;
        camera.farPlane = farClip;
        camera.orthographicSize = 10.0f;
        camera.viewMatrix = glm::mat4(1.0f);
        camera.projectionMatrix = glm::perspective(glm::radians(fov), aspect, nearClip, farClip);
        return camera;
    }

    Camera Camera::CreateOrthographic(float size, float aspect, float nearClip, float farClip)
    {
        Camera camera;
        camera.projectionType = ProjectionType::ORTHOGRAPHIC;
        camera.orthographicSize = size;
        camera.aspectRatio = aspect;
        camera.nearPlane = nearClip;
        camera.farPlane = farClip;
        camera.fieldOfView = 60.0f;
        camera.viewMatrix = glm::mat4(1.0f);

        float halfHeight = size;
        float halfWidth = halfHeight * aspect;
        camera.projectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearClip, farClip);

        return camera;
    }

    glm::mat4 Camera::GetViewMatrix() const
    {
        return viewMatrix;
    }

    glm::mat4 Camera::GetProjectionMatrix() const
    {
        return projectionMatrix;
    }

    glm::mat4 Camera::GetViewProjectionMatrix() const
    {
        return projectionMatrix * viewMatrix;
    }
}
