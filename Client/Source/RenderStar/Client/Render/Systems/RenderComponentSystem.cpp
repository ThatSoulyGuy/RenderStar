#include "RenderStar/Client/Render/Systems/RenderComponentSystem.hpp"

namespace RenderStar::Client::Render::Systems
{
    RenderComponentSystem::RenderComponentSystem()
        : renderCallback(nullptr)
        , activeCameraEntity{ GameObject::INVALID_ID }
        , activeCamera{}
        , viewMatrix(1.0f)
        , projectionMatrix(1.0f)
        , hasActiveCamera(false)
    {
    }

    void RenderComponentSystem::SetRenderCallback(RenderCallback callback)
    {
        renderCallback = std::move(callback);
    }

    void RenderComponentSystem::Run(ComponentModule& ecs)
    {
        ClearRenderCommands();
        CollectCameras(ecs);
        CollectRenderables(ecs);
    }

    const std::vector<RenderCommand>& RenderComponentSystem::GetRenderCommands() const
    {
        return renderCommands;
    }

    void RenderComponentSystem::ClearRenderCommands()
    {
        renderCommands.clear();
    }

    bool RenderComponentSystem::HasActiveCamera() const
    {
        return hasActiveCamera;
    }

    const Camera& RenderComponentSystem::GetActiveCamera() const
    {
        return activeCamera;
    }

    const glm::mat4& RenderComponentSystem::GetViewMatrix() const
    {
        return viewMatrix;
    }

    const glm::mat4& RenderComponentSystem::GetProjectionMatrix() const
    {
        return projectionMatrix;
    }

    void RenderComponentSystem::SetActiveCamera(GameObject cameraEntity)
    {
        activeCameraEntity = cameraEntity;
    }

    void RenderComponentSystem::CollectCameras(ComponentModule& ecs)
    {
        hasActiveCamera = false;

        auto& cameraPool = ecs.GetPool<Camera>();

        if (cameraPool.GetSize() == 0)
            return;

        if (activeCameraEntity.IsValid() && ecs.HasComponent<Camera>(activeCameraEntity))
        {
            auto cameraOpt = ecs.GetComponent<Camera>(activeCameraEntity);

            if (cameraOpt.has_value())
            {
                activeCamera = cameraOpt.value().get();
                hasActiveCamera = true;
            }
        }
        else
        {
            for (auto [entity, camera] : cameraPool)
            {
                activeCamera = camera;
                activeCameraEntity = entity;
                hasActiveCamera = true;
                break;
            }
        }

        if (hasActiveCamera)
        {
            viewMatrix = activeCamera.GetViewMatrix();
            projectionMatrix = activeCamera.GetProjectionMatrix();
        }
    }

    void RenderComponentSystem::CollectRenderables(ComponentModule& ecs)
    {
        auto& meshRendererPool = ecs.GetPool<MeshRenderer>();

        if (meshRendererPool.GetSize() == 0)
            return;

        for (auto [entity, renderer] : meshRendererPool)
        {
            if (!renderer.visible || !renderer.IsValid())
                continue;

            if (!ecs.HasComponent<Transform>(entity))
                continue;

            auto transformOpt = ecs.GetComponent<Transform>(entity);

            if (!transformOpt.has_value())
                continue;

            Transform& transform = transformOpt.value().get();

            RenderCommand command{};
            command.entity = entity;
            command.modelMatrix = transform.worldMatrix;
            command.meshHandle = renderer.meshHandle;
            command.materialHandle = renderer.materialHandle;

            renderCommands.push_back(command);

            if (renderCallback)
                renderCallback(command);
        }
    }
}
