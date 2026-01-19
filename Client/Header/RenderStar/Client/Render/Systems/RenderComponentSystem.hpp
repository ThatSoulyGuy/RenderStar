#pragma once

#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Client/Render/Components/Camera.hpp"
#include "RenderStar/Client/Render/Components/MeshRenderer.hpp"
#include "RenderStar/Client/Render/Resource/IMesh.hpp"
#include "RenderStar/Client/Render/Resource/IMaterial.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <functional>

namespace RenderStar::Client::Render::Systems
{
    using namespace RenderStar::Common::Component;
    using namespace RenderStar::Client::Render::Components;

    struct RenderCommand
    {
        GameObject entity;
        glm::mat4 modelMatrix;
        MeshHandle meshHandle;
        MaterialHandle materialHandle;
    };

    using RenderCallback = std::function<void(const RenderCommand&)>;

    class RenderComponentSystem
    {
    public:

        RenderComponentSystem();

        void SetRenderCallback(RenderCallback callback);

        void Run(ComponentModule& ecs);

        const std::vector<RenderCommand>& GetRenderCommands() const;

        void ClearRenderCommands();

        bool HasActiveCamera() const;

        const Camera& GetActiveCamera() const;

        const glm::mat4& GetViewMatrix() const;

        const glm::mat4& GetProjectionMatrix() const;

        void SetActiveCamera(GameObject cameraEntity);

    private:

        void CollectCameras(ComponentModule& ecs);
        void CollectRenderables(ComponentModule& ecs);

        RenderCallback renderCallback;
        std::vector<RenderCommand> renderCommands;
        GameObject activeCameraEntity;
        Camera activeCamera;
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        bool hasActiveCamera;
    };
}
