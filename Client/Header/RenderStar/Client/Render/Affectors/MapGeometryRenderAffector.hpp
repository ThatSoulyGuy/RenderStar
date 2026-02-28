#pragma once

#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include "RenderStar/Client/Render/Resource/ITextureHandle.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/Mesh.hpp"
#include "RenderStar/Common/Component/AbstractAffector.hpp"
#include "RenderStar/Common/Scene/MapbinLoader.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace RenderStar::Common::Scene { class SceneModule; }
namespace RenderStar::Common::Asset { class AssetModule; }

namespace RenderStar::Client::Render
{
    class IRenderBackend;
    class IBufferManager;
    class IUniformManager;
    class ITextureManager;
}

namespace RenderStar::Client::Render::Affectors
{
    class MapGeometryRenderAffector final : public Common::Component::AbstractAffector
    {
    public:

        void Affect(Common::Component::ComponentModule& componentModule) override;

        void SetupRenderState(IBufferManager* bufferManager, IUniformManager* uniformManager, ITextureManager* textureManager);
        void SetShader(std::unique_ptr<IShaderProgram> shader);

        void CheckForNewMapGeometry(Common::Component::ComponentModule& componentModule);

        void BuildScene(
            const std::vector<Common::Scene::MapbinGroup>& groups,
            const std::vector<Common::Scene::MapbinTexture>& textures,
            Common::Scene::SceneModule& sceneModule,
            Common::Component::ComponentModule& componentModule);

        void Render(Common::Component::ComponentModule& componentModule, IRenderBackend* backend, const glm::mat4& viewProjection);
        void Cleanup();

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        struct UniformSlot
        {
            std::unique_ptr<IBufferHandle> buffer;
            std::unique_ptr<IUniformBindingHandle> binding;
        };

        UniformSlot& AcquireUniformSlot();

        std::unique_ptr<IShaderProgram> shader;
        IBufferManager* bufferManager = nullptr;
        IUniformManager* uniformManager = nullptr;
        ITextureManager* textureManager = nullptr;
        std::unordered_map<int32_t, ITextureHandle*> materialTextures;
        std::vector<UniformSlot> uniformPool;
        size_t uniformPoolIndex = 0;

        std::vector<std::unique_ptr<Resource::Mesh>> sceneMeshes;
        std::vector<std::unique_ptr<ITextureHandle>> sceneTextures;

        Common::Scene::SceneModule* sceneModule = nullptr;
        Common::Asset::AssetModule* assetModule = nullptr;
        std::unordered_set<int32_t> processedMapEntities;
        bool guardWarningLogged = false;
    };
}
