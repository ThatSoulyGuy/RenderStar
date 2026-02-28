#pragma once

#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/Mesh.hpp"
#include "RenderStar/Common/Component/AbstractAffector.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace RenderStar::Client::Render
{
    class IRenderBackend;
    class IBufferManager;
    class IUniformManager;
    class ITextureManager;
}

namespace RenderStar::Client::Render::Affectors
{
    class PlayerRenderAffector final : public Common::Component::AbstractAffector
    {
    public:

        void Affect(Common::Component::ComponentModule& componentModule) override;

        void SetupRenderState(IBufferManager* bufferManager, IUniformManager* uniformManager, ITextureManager* textureManager);
        void SetShader(std::unique_ptr<IShaderProgram> shader);

        void Render(Common::Component::ComponentModule& componentModule, IRenderBackend* backend,
            const glm::mat4& viewProjection, int32_t localPlayerId);

    private:

        struct UniformSlot
        {
            std::unique_ptr<IBufferHandle> buffer;
            std::unique_ptr<IUniformBindingHandle> binding;
        };

        UniformSlot& AcquireUniformSlot();
        void BuildCubeMesh();

        std::unique_ptr<IShaderProgram> shader;
        std::unique_ptr<Resource::Mesh> cubeMesh;
        IBufferManager* bufferManager = nullptr;
        IUniformManager* uniformManager = nullptr;
        ITextureManager* textureManager = nullptr;
        std::vector<UniformSlot> uniformPool;
        size_t uniformPoolIndex = 0;
    };
}
