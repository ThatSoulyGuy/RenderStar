#include "RenderStar/Client/Render/Systems/CameraComponentSystem.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Client/Render/Components/Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace RenderStar::Client::Render::Systems
{
    using namespace RenderStar::Common::Component;
    using namespace RenderStar::Client::Render::Components;

    CameraComponentSystem::CameraComponentSystem()
        : viewportWidth(1280)
        , viewportHeight(720)
    {
    }

    void CameraComponentSystem::SetViewportSize(int32_t width, int32_t height)
    {
        viewportWidth = width;
        viewportHeight = height;
    }

    void CameraComponentSystem::Run(ComponentModule& componentModule)
    {
        float defaultAspect = 16.0f / 9.0f;

        if (viewportHeight > 0)
            defaultAspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);

        auto& cameraPool = componentModule.GetPool<Camera>();

        if (cameraPool.GetSize() == 0)
            return;

        for (auto [entity, camera] : cameraPool)
        {
            if (!componentModule.HasComponent<Transform>(entity))
                continue;

            auto transformOpt = componentModule.GetComponent<Transform>(entity);

            if (!transformOpt.has_value())
                continue;

            Transform& transform = transformOpt.value().get();

            float posX = transform.worldMatrix[3][0];
            float posY = transform.worldMatrix[3][1];
            float posZ = transform.worldMatrix[3][2];

            glm::vec3 forward = glm::normalize(glm::vec3(
                -transform.worldMatrix[2][0],
                -transform.worldMatrix[2][1],
                -transform.worldMatrix[2][2]
            ));

            glm::vec3 up = glm::normalize(glm::vec3(
                transform.worldMatrix[1][0],
                transform.worldMatrix[1][1],
                transform.worldMatrix[1][2]
            ));

            glm::vec3 position(posX, posY, posZ);
            glm::vec3 target = position + forward;

            camera.viewMatrix = glm::lookAt(position, target, up);

            float aspect = camera.aspectRatio > 0 ? camera.aspectRatio : defaultAspect;

            switch (camera.projectionType)
            {
                case ProjectionType::PERSPECTIVE:
                {
                    float fovRadians = glm::radians(camera.fieldOfView);
                    camera.projectionMatrix = glm::perspective(
                        fovRadians,
                        aspect,
                        camera.nearPlane,
                        camera.farPlane
                    );
                    break;
                }
                case ProjectionType::ORTHOGRAPHIC:
                {
                    float halfHeight = camera.orthographicSize;
                    float halfWidth = halfHeight * aspect;
                    camera.projectionMatrix = glm::ortho(
                        -halfWidth, halfWidth,
                        -halfHeight, halfHeight,
                        camera.nearPlane,
                        camera.farPlane
                    );
                    break;
                }
            }

            camera.projectionMatrix[1][1] *= -1.0f;
        }
    }
}
