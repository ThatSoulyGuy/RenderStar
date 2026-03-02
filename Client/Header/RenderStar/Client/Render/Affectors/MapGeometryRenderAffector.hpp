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

        struct LoadedMaterial
        {
            float roughness = 0.5f;
            float metallic = 0.0f;
            float specularStrength = 0.5f;
            float normalStrength = 1.0f;
            float aoStrength = 1.0f;
            float emissionStrength = 1.0f;
            float detailScale = 1.0f;
            ITextureHandle* baseColor = nullptr;
            ITextureHandle* normalMap = nullptr;
            ITextureHandle* roughnessMap = nullptr;
            ITextureHandle* metallicMap = nullptr;
            ITextureHandle* aoMap = nullptr;
            ITextureHandle* emissionMap = nullptr;
            ITextureHandle* specularMap = nullptr;
            ITextureHandle* detailAlbedoMap = nullptr;
            ITextureHandle* detailNormalMap = nullptr;
        };

        void Affect(Common::Component::ComponentModule& componentModule) override;

        void SetupRenderState(IBufferManager* bufferManager, IUniformManager* uniformManager, ITextureManager* textureManager);
        void SetShader(std::unique_ptr<IShaderProgram> shader);

        void CheckForNewMapGeometry(Common::Component::ComponentModule& componentModule);

        void BuildScene(
            const std::vector<Common::Scene::MapbinGroup>& groups,
            const std::vector<Common::Scene::MapbinMaterial>& materials,
            const std::vector<Common::Scene::MapbinGameObject>& gameObjects,
            Common::Scene::SceneModule& sceneModule,
            Common::Component::ComponentModule& componentModule);

        void SetSceneLightingBuffer(IBufferHandle* buffer);
        void SetShadowShader(std::unique_ptr<IShaderProgram> shader);
        void SetShadowMapTexture(ITextureHandle* texture);
        void Render(Common::Component::ComponentModule& componentModule, IRenderBackend* backend, const glm::mat4& viewProjection);
        void RenderShadowDepth(Common::Component::ComponentModule& componentModule, IRenderBackend* backend, const glm::mat4& lightViewProjection);
        void Cleanup();

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        struct UniformSlot
        {
            std::unique_ptr<IBufferHandle> buffer;
            std::unique_ptr<IBufferHandle> materialBuffer;
            std::unique_ptr<IUniformBindingHandle> binding;
        };

        struct ShadowUniformSlot
        {
            std::unique_ptr<IBufferHandle> buffer;
            std::unique_ptr<IUniformBindingHandle> binding;
        };

        UniformSlot& AcquireUniformSlot();
        ShadowUniformSlot& AcquireShadowUniformSlot();

        std::unique_ptr<IShaderProgram> shader;
        std::unique_ptr<IShaderProgram> shadowShader;
        std::vector<ShadowUniformSlot> shadowUniformPool;
        size_t shadowUniformPoolIndex = 0;
        ITextureHandle* shadowMapTexture = nullptr;
        IBufferManager* bufferManager = nullptr;
        IUniformManager* uniformManager = nullptr;
        ITextureManager* textureManager = nullptr;
        std::unordered_map<int32_t, LoadedMaterial> loadedMaterials;
        std::vector<UniformSlot> uniformPool;
        size_t uniformPoolIndex = 0;

        std::vector<std::unique_ptr<Resource::Mesh>> sceneMeshes;
        std::vector<std::unique_ptr<ITextureHandle>> sceneTextures;

        IBufferHandle* sceneLightingBuffer = nullptr;

        Common::Scene::SceneModule* sceneModule = nullptr;
        Common::Asset::AssetModule* assetModule = nullptr;
        std::unordered_set<int32_t> processedMapEntities;
        bool guardWarningLogged = false;
    };
}
