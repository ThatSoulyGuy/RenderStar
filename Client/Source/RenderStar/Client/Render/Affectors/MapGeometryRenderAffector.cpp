#include "RenderStar/Client/Render/Affectors/MapGeometryRenderAffector.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Components/MapbinMesh.hpp"
#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Client/Render/Resource/ITextureManager.hpp"
#include "RenderStar/Client/Render/Resource/IUniformManager.hpp"
#include "RenderStar/Client/Render/Resource/StandardUniforms.hpp"
#include "RenderStar/Client/Render/Resource/Vertex.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Asset/AssetLocation.hpp"
#include "RenderStar/Common/Asset/IBinaryAsset.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/MapGeometry.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Scene/SceneModule.hpp"

namespace RenderStar::Client::Render::Affectors
{
    namespace
    {
        TextureWrapMode GlWrapToWrapMode(uint32_t glWrap)
        {
            if (glWrap == 0x812F)
                return TextureWrapMode::CLAMP_TO_EDGE;
            return TextureWrapMode::REPEAT;
        }

        TextureFilterMode GlFilterToFilterMode(uint32_t glFilter)
        {
            if (glFilter == 0x2600)
                return TextureFilterMode::NEAREST;
            return TextureFilterMode::LINEAR;
        }
    }

    void MapGeometryRenderAffector::OnInitialize(Common::Module::ModuleContext& context)
    {
        auto sceneOpt = context.GetModule<Common::Scene::SceneModule>();

        if (sceneOpt.has_value())
            sceneModule = &sceneOpt->get();
        else
            logger->error("MapGeometryRenderAffector: SceneModule not found");

        auto assetOpt = context.GetModule<Common::Asset::AssetModule>();

        if (assetOpt.has_value())
            assetModule = &assetOpt->get();
        else
            logger->error("MapGeometryRenderAffector: AssetModule not found");

        logger->info("MapGeometryRenderAffector initialized: sceneModule={}, assetModule={}",
            static_cast<void*>(sceneModule), static_cast<void*>(assetModule));
    }

    void MapGeometryRenderAffector::Affect(Common::Component::ComponentModule&)
    {
    }

    void MapGeometryRenderAffector::SetupRenderState(IBufferManager* bm, IUniformManager* um, ITextureManager* tm)
    {
        bufferManager = bm;
        uniformManager = um;
        textureManager = tm;

        logger->info("MapGeometryRenderAffector::SetupRenderState: bufferManager={}, uniformManager={}, textureManager={}",
            static_cast<void*>(bufferManager), static_cast<void*>(uniformManager), static_cast<void*>(textureManager));
    }

    void MapGeometryRenderAffector::SetShader(std::unique_ptr<IShaderProgram> s)
    {
        shader = std::move(s);
    }

    void MapGeometryRenderAffector::CheckForNewMapGeometry(Common::Component::ComponentModule& componentModule)
    {
        if (!sceneModule || !assetModule || !bufferManager)
        {
            if (!guardWarningLogged)
            {
                logger->warn("CheckForNewMapGeometry guard failed: sceneModule={}, assetModule={}, bufferManager={}",
                    static_cast<void*>(sceneModule), static_cast<void*>(assetModule), static_cast<void*>(bufferManager));
                guardWarningLogged = true;
            }

            return;
        }

        auto& pool = componentModule.GetPool<Common::Component::MapGeometry>();

        for (auto [entity, mapGeometry] : pool)
        {
            if (processedMapEntities.contains(entity.id))
                continue;

            logger->info("CheckForNewMapGeometry: found new MapGeometry entity id={}, assetPath='{}'", entity.id, mapGeometry.assetPath);
            processedMapEntities.insert(entity.id);

            auto binaryAsset = assetModule->LoadBinary(Common::Asset::AssetLocation::Parse(mapGeometry.assetPath));

            if (!binaryAsset.IsValid())
            {
                logger->error("Failed to load mapbin asset: {}", mapGeometry.assetPath);
                continue;
            }

            auto scene = Common::Scene::MapbinLoader::Load(binaryAsset.Get()->GetDataView());

            if (!scene.has_value())
            {
                logger->error("Failed to parse mapbin: {}", mapGeometry.assetPath);
                continue;
            }

            BuildScene(scene->groups, scene->textures, *sceneModule, componentModule);

            logger->info("Loaded map geometry from '{}': {} groups, {} textures",
                mapGeometry.assetPath, scene->groups.size(), scene->textures.size());
        }
    }

    void MapGeometryRenderAffector::BuildScene(
        const std::vector<Common::Scene::MapbinGroup>& groups,
        const std::vector<Common::Scene::MapbinTexture>& textures,
        Common::Scene::SceneModule& sceneModule,
        Common::Component::ComponentModule& componentModule)
    {
        sceneMeshes.clear();
        sceneTextures.clear();
        materialTextures.clear();

        if (textureManager)
        {
            for (const auto& tex : textures)
            {
                TextureDescription desc;
                desc.width = tex.width;
                desc.height = tex.height;
                desc.wrapS = GlWrapToWrapMode(tex.wrapS);
                desc.wrapT = GlWrapToWrapMode(tex.wrapT);
                desc.minFilter = GlFilterToFilterMode(tex.minFilter);
                desc.magFilter = GlFilterToFilterMode(tex.magFilter);

                auto handle = textureManager->CreateFromMemory(desc, tex.pixelData.data());

                if (handle && handle->IsValid())
                {
                    materialTextures[tex.materialId] = handle.get();
                    sceneTextures.push_back(std::move(handle));
                }
            }
        }

        for (const auto& group : groups)
        {
            auto mesh = std::make_unique<Resource::Mesh>(*bufferManager, Vertex::LAYOUT, PrimitiveType::TRIANGLES);

            mesh->SetVertexData(group.vertexData.data(), group.vertexData.size() * sizeof(float));
            mesh->SetIndexData(group.indices.data(), group.indices.size() * sizeof(uint32_t), IndexType::UINT32);

            auto entity = sceneModule.CreateEntity();

            auto& transform = componentModule.AddComponent<Common::Component::Transform>(entity);
            transform.scale = glm::vec3(0.1f);

            auto& mapbinMesh = componentModule.AddComponent<Components::MapbinMesh>(entity);
            mapbinMesh.mesh = mesh.get();
            mapbinMesh.materialId = group.materialId;

            sceneMeshes.push_back(std::move(mesh));
        }
    }

    void MapGeometryRenderAffector::Render(Common::Component::ComponentModule& componentModule, IRenderBackend* backend, const glm::mat4& viewProjection)
    {
        if (!backend || !shader || !shader->IsValid() || !bufferManager || !uniformManager)
            return;

        int32_t frameIndex = backend->GetCurrentFrame();
        uniformPoolIndex = 0;

        auto& pool = componentModule.GetPool<Components::MapbinMesh>();

        for (auto [entity, mapbinMesh] : pool)
        {
            if (!mapbinMesh.mesh || !mapbinMesh.mesh->IsValid())
                continue;

            auto transformOpt = componentModule.GetComponent<Common::Component::Transform>(entity);

            if (!transformOpt.has_value())
                continue;

            auto& transform = transformOpt->get();

            StandardUniforms uniforms(transform.worldMatrix, viewProjection, glm::vec4(0.0f));

            auto& slot = AcquireUniformSlot();

            slot.buffer->SetSubData(&uniforms, StandardUniforms::Size(), 0);

            ITextureHandle* texture = textureManager ? textureManager->GetDefaultTexture() : nullptr;

            auto it = materialTextures.find(mapbinMesh.materialId);

            if (it != materialTextures.end())
                texture = it->second;

            if (texture && slot.binding)
                slot.binding->UpdateTexture(1, texture, frameIndex);

            if (slot.binding)
                backend->SubmitDrawCommand(shader.get(), slot.binding.get(), frameIndex, mapbinMesh.mesh->GetUnderlyingMesh());
        }

        backend->ExecuteDrawCommands();
    }

    void MapGeometryRenderAffector::Cleanup()
    {
        sceneMeshes.clear();
        sceneTextures.clear();
        materialTextures.clear();
        uniformPool.clear();
        uniformPoolIndex = 0;
        processedMapEntities.clear();
        shader.reset();
        bufferManager = nullptr;
        uniformManager = nullptr;
        textureManager = nullptr;
    }

    MapGeometryRenderAffector::UniformSlot& MapGeometryRenderAffector::AcquireUniformSlot()
    {
        if (uniformPoolIndex < uniformPool.size())
            return uniformPool[uniformPoolIndex++];

        UniformSlot slot;
        slot.buffer = bufferManager->CreateUniformBuffer(StandardUniforms::Size());
        slot.binding = uniformManager->CreateBindingForShader(shader.get());

        if (slot.binding)
        {
            slot.binding->UpdateBuffer(0, slot.buffer.get(), StandardUniforms::Size());

            if (textureManager)
                slot.binding->UpdateTexture(1, textureManager->GetDefaultTexture());
        }

        uniformPool.push_back(std::move(slot));
        return uniformPool[uniformPoolIndex++];
    }
}
