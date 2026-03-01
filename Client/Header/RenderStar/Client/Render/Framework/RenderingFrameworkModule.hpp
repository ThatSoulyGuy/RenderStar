#pragma once

#include "RenderStar/Client/Render/Framework/SceneLightingData.hpp"
#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace RenderStar::Common::Component { class ComponentModule; }

namespace RenderStar::Client::Render
{
    class IBufferManager;
}

namespace RenderStar::Client::Render::Framework
{
    class RenderingFrameworkModule final : public Common::Module::AbstractModule
    {
    public:

        void SetupRenderState(IBufferManager* bufferManager);
        void CollectSceneData(Common::Component::ComponentModule& componentModule,
                              const glm::vec3& cameraPosition);
        IBufferHandle* GetSceneLightingBuffer() const;
        glm::mat4 GetLightViewProjection() const;
        void SetAmbientLight(glm::vec3 color, float intensity);
        void Cleanup();

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        std::unique_ptr<IBufferHandle> sceneLightingBuffer;
        SceneLightingData sceneLightingData{};
        glm::vec3 ambientColor = glm::vec3(0.15f);
        float ambientIntensity = 1.0f;
        IBufferManager* bufferManager = nullptr;
    };
}
