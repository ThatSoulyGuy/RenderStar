#pragma once

#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/Mesh.hpp"
#include "RenderStar/Common/Component/AbstractAffector.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace RenderStar::Client::Render
{
    class IRenderBackend;
    class IBufferManager;
    class IUniformManager;
    class ITextureManager;
}

namespace RenderStar::Client::Render::Affectors
{
    class SkyboxRenderAffector final : public Common::Component::AbstractAffector
    {
    public:

        struct SkyUniforms
        {
            glm::mat4 viewProjectionNoTranslation;
            glm::vec4 sunDirection;
            glm::vec4 sunColor;
            glm::vec4 ambientColor;
            glm::vec4 skyParams;

            static constexpr size_t Size() { return sizeof(SkyUniforms); }
        };

        void Affect(Common::Component::ComponentModule& componentModule) override;

        void SetupRenderState(IBufferManager* bufferManager, IUniformManager* uniformManager);
        void SetShader(std::unique_ptr<IShaderProgram> shader);
        void Cleanup();

        void Render(IRenderBackend* backend,
            const glm::mat4& viewMatrix,
            const glm::mat4& projectionMatrix,
            const glm::vec3& sunDirection,
            const glm::vec3& sunColor,
            float sunIntensity,
            const glm::vec3& ambientColor,
            float ambientIntensity);

    private:

        void BuildCubeMesh();

        std::unique_ptr<IShaderProgram> shader;
        std::unique_ptr<Resource::Mesh> cubeMesh;
        std::unique_ptr<IBufferHandle> uniformBuffer;
        std::unique_ptr<IUniformBindingHandle> binding;
        IBufferManager* bufferManager = nullptr;
        IUniformManager* uniformManager = nullptr;
    };
}
