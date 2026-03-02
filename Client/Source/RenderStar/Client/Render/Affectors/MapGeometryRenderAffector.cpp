#include "RenderStar/Client/Render/Affectors/MapGeometryRenderAffector.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Components/Light.hpp"
#include "RenderStar/Client/Render/Components/MapbinMesh.hpp"
#include "RenderStar/Client/Render/Framework/LitVertex.hpp"
#include "RenderStar/Client/Render/Framework/SceneLightingData.hpp"
#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Client/Render/Resource/ITextureManager.hpp"
#include "RenderStar/Client/Render/Resource/IUniformManager.hpp"
#include "RenderStar/Client/Render/Resource/MaterialProperties.hpp"
#include "RenderStar/Client/Render/Resource/StandardUniforms.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Asset/AssetLocation.hpp"
#include "RenderStar/Common/Asset/IBinaryAsset.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/MapGeometry.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include <glm/gtc/matrix_transform.hpp>

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

        glm::vec3 EulerToDirection(float rotXDeg, float rotYDeg, float rotZDeg)
        {
            glm::mat4 rot(1.0f);
            rot = glm::rotate(rot, glm::radians(rotYDeg), glm::vec3(0.0f, 1.0f, 0.0f));
            rot = glm::rotate(rot, glm::radians(rotXDeg), glm::vec3(1.0f, 0.0f, 0.0f));
            rot = glm::rotate(rot, glm::radians(rotZDeg), glm::vec3(0.0f, 0.0f, 1.0f));
            return glm::normalize(glm::vec3(rot * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
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

    void MapGeometryRenderAffector::SetSceneLightingBuffer(IBufferHandle* buffer)
    {
        sceneLightingBuffer = buffer;
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

            BuildScene(scene->groups, scene->materials, scene->gameObjects, *sceneModule, componentModule);

            logger->info("Loaded map geometry from '{}': {} groups, {} materials",
                mapGeometry.assetPath, scene->groups.size(), scene->materials.size());
        }
    }

    void MapGeometryRenderAffector::BuildScene(
        const std::vector<Common::Scene::MapbinGroup>& groups,
        const std::vector<Common::Scene::MapbinMaterial>& materials,
        const std::vector<Common::Scene::MapbinGameObject>& gameObjects,
        Common::Scene::SceneModule& sceneModule,
        Common::Component::ComponentModule& componentModule)
    {
        sceneMeshes.clear();
        sceneTextures.clear();
        loadedMaterials.clear();

        if (textureManager)
        {
            for (const auto& mat : materials)
            {
                LoadedMaterial loaded;
                loaded.roughness = mat.roughness;
                loaded.metallic = mat.metallic;
                loaded.specularStrength = mat.specularStrength;
                loaded.normalStrength = mat.normalStrength;
                loaded.aoStrength = mat.aoStrength;
                loaded.emissionStrength = mat.emissionStrength;
                loaded.detailScale = mat.detailScale;

                for (const auto& slot : mat.textureSlots)
                {
                    TextureDescription desc;
                    desc.width = slot.width;
                    desc.height = slot.height;
                    desc.wrapS = GlWrapToWrapMode(slot.wrapS);
                    desc.wrapT = GlWrapToWrapMode(slot.wrapT);
                    desc.minFilter = GlFilterToFilterMode(slot.minFilter);
                    desc.magFilter = GlFilterToFilterMode(slot.magFilter);

                    auto handle = textureManager->CreateFromMemory(desc, slot.pixelData.data());

                    if (handle && handle->IsValid())
                    {
                        ITextureHandle* rawPtr = handle.get();

                        switch (slot.slotType)
                        {
                            case Common::Scene::TextureSlotType::BASE_COLOR:
                                loaded.baseColor = rawPtr;
                                break;
                            case Common::Scene::TextureSlotType::NORMAL:
                                loaded.normalMap = rawPtr;
                                break;
                            case Common::Scene::TextureSlotType::ROUGHNESS:
                                loaded.roughnessMap = rawPtr;
                                break;
                            case Common::Scene::TextureSlotType::METALLIC:
                                loaded.metallicMap = rawPtr;
                                break;
                            case Common::Scene::TextureSlotType::AO:
                                loaded.aoMap = rawPtr;
                                break;
                            case Common::Scene::TextureSlotType::EMISSION:
                                loaded.emissionMap = rawPtr;
                                break;
                            case Common::Scene::TextureSlotType::SPECULAR:
                                loaded.specularMap = rawPtr;
                                break;
                            case Common::Scene::TextureSlotType::DETAIL_ALBEDO:
                                loaded.detailAlbedoMap = rawPtr;
                                break;
                            case Common::Scene::TextureSlotType::DETAIL_NORMAL:
                                loaded.detailNormalMap = rawPtr;
                                break;
                            default:
                                break;
                        }

                        sceneTextures.push_back(std::move(handle));
                    }
                }

                loadedMaterials[mat.materialId] = loaded;
            }
        }

        for (const auto& group : groups)
        {
            size_t vertexCount = group.vertexData.size() / 8;
            const float* raw = group.vertexData.data();

            std::vector<glm::vec3> tangentAccum(vertexCount, glm::vec3(0.0f));

            for (size_t i = 0; i + 2 < group.indices.size(); i += 3)
            {
                uint32_t i0 = group.indices[i];
                uint32_t i1 = group.indices[i + 1];
                uint32_t i2 = group.indices[i + 2];

                glm::vec3 p0(raw[i0 * 8], raw[i0 * 8 + 1], raw[i0 * 8 + 2]);
                glm::vec3 p1(raw[i1 * 8], raw[i1 * 8 + 1], raw[i1 * 8 + 2]);
                glm::vec3 p2(raw[i2 * 8], raw[i2 * 8 + 1], raw[i2 * 8 + 2]);

                glm::vec2 uv0(raw[i0 * 8 + 6], raw[i0 * 8 + 7]);
                glm::vec2 uv1(raw[i1 * 8 + 6], raw[i1 * 8 + 7]);
                glm::vec2 uv2(raw[i2 * 8 + 6], raw[i2 * 8 + 7]);

                glm::vec3 e1 = p1 - p0;
                glm::vec3 e2 = p2 - p0;
                glm::vec2 dUV1 = uv1 - uv0;
                glm::vec2 dUV2 = uv2 - uv0;

                float denom = dUV1.x * dUV2.y - dUV2.x * dUV1.y;
                float f = (denom * denom > 1e-16f) ? 1.0f / denom : 0.0f;

                glm::vec3 tangent = f * (dUV2.y * e1 - dUV1.y * e2);

                tangentAccum[i0] += tangent;
                tangentAccum[i1] += tangent;
                tangentAccum[i2] += tangent;
            }

            std::vector<Framework::LitVertex> vertices(vertexCount);

            for (size_t v = 0; v < vertexCount; ++v)
            {
                size_t base = v * 8;
                glm::vec3 normal(raw[base + 3], raw[base + 4], raw[base + 5]);
                glm::vec3 t = tangentAccum[v];
                t = t - normal * glm::dot(normal, t);
                float len = glm::length(t);
                t = (len > 1e-6f) ? t / len : glm::vec3(1.0f, 0.0f, 0.0f);

                vertices[v] = Framework::LitVertex(
                    raw[base], raw[base + 1], raw[base + 2],
                    raw[base + 3], raw[base + 4], raw[base + 5],
                    raw[base + 6], raw[base + 7],
                    t.x, t.y, t.z);
            }

            auto mesh = std::make_unique<Resource::Mesh>(*bufferManager, Framework::LitVertex::LAYOUT, PrimitiveType::TRIANGLES);

            mesh->SetVertices(vertices);
            mesh->SetIndexData(group.indices.data(), group.indices.size() * sizeof(uint32_t), IndexType::UINT32);

            auto entity = sceneModule.CreateEntity();

            auto& transform = componentModule.AddComponent<Common::Component::Transform>(entity);
            transform.scale = glm::vec3(0.1f);
            transform.localMatrix = glm::scale(glm::mat4(1.0f), transform.scale);
            transform.worldMatrix = transform.localMatrix;
            transform.worldScale = transform.scale;

            auto& mapbinMesh = componentModule.AddComponent<Components::MapbinMesh>(entity);
            mapbinMesh.mesh = mesh.get();
            mapbinMesh.materialId = group.materialId;

            sceneMeshes.push_back(std::move(mesh));
        }

        for (const auto& obj : gameObjects)
        {
            switch (obj.type)
            {
                case Common::Scene::GameObjectType::POINT_LIGHT:
                {
                    auto lightEntity = sceneModule.CreateEntity();
                    auto& transform = componentModule.AddComponent<Common::Component::Transform>(lightEntity);
                    transform.position = glm::vec3(obj.posX, obj.posY, obj.posZ) * 0.1f;

                    componentModule.AddComponent<Components::Light>(lightEntity,
                        Components::Light::Point(
                            glm::vec3(obj.colorR, obj.colorG, obj.colorB),
                            obj.intensity,
                            50.0f));
                    break;
                }
                case Common::Scene::GameObjectType::SPOT_LIGHT:
                {
                    glm::vec3 dir = EulerToDirection(obj.rotX, obj.rotY, obj.rotZ);
                    float softness = (obj.outerCone > 0.0f)
                        ? 1.0f - (obj.innerCone / obj.outerCone) : 0.0f;

                    auto lightEntity = sceneModule.CreateEntity();
                    auto& transform = componentModule.AddComponent<Common::Component::Transform>(lightEntity);
                    transform.position = glm::vec3(obj.posX, obj.posY, obj.posZ) * 0.1f;

                    auto light = Components::Light::Spot(
                        dir,
                        glm::vec3(obj.colorR, obj.colorG, obj.colorB),
                        obj.intensity,
                        50.0f,
                        obj.outerCone);
                    light.spotSoftness = softness;

                    componentModule.AddComponent<Components::Light>(lightEntity, light);
                    break;
                }
                case Common::Scene::GameObjectType::SUN_LIGHT:
                {
                    glm::vec3 dir = -EulerToDirection(obj.rotX, obj.rotY, obj.rotZ);

                    auto lightEntity = sceneModule.CreateEntity();

                    componentModule.AddComponent<Components::Light>(lightEntity,
                        Components::Light::Directional(
                            dir,
                            glm::vec3(obj.colorR, obj.colorG, obj.colorB),
                            obj.intensity));
                    break;
                }
                default:
                    break;
            }
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

            ITextureHandle* defaultTex = textureManager ? textureManager->GetDefaultTexture() : nullptr;
            ITextureHandle* baseColorTex = defaultTex;
            ITextureHandle* roughnessTex = defaultTex;
            ITextureHandle* metallicTex = defaultTex;
            ITextureHandle* aoTex = defaultTex;
            ITextureHandle* emissionTex = defaultTex;
            ITextureHandle* normalTex = defaultTex;
            ITextureHandle* specularTex = defaultTex;
            ITextureHandle* detailAlbedoTex = defaultTex;
            ITextureHandle* detailNormalTex = defaultTex;
            MaterialProperties matProps;

            auto it = loadedMaterials.find(mapbinMesh.materialId);

            if (it != loadedMaterials.end())
            {
                const auto& mat = it->second;

                if (mat.baseColor)
                    baseColorTex = mat.baseColor;
                if (mat.roughnessMap)
                    roughnessTex = mat.roughnessMap;
                if (mat.metallicMap)
                    metallicTex = mat.metallicMap;
                if (mat.aoMap)
                    aoTex = mat.aoMap;
                if (mat.emissionMap)
                    emissionTex = mat.emissionMap;
                if (mat.normalMap)
                    normalTex = mat.normalMap;
                if (mat.specularMap)
                    specularTex = mat.specularMap;
                if (mat.detailAlbedoMap)
                    detailAlbedoTex = mat.detailAlbedoMap;
                if (mat.detailNormalMap)
                    detailNormalTex = mat.detailNormalMap;

                float effectiveEmission = mat.emissionMap ? mat.emissionStrength : 0.0f;
                float effectiveNormal = mat.normalMap ? mat.normalStrength : 0.0f;
                float effectiveSpecular = mat.specularMap ? mat.specularStrength : 0.5f;
                bool hasDetail = mat.detailAlbedoMap || mat.detailNormalMap;
                float effectiveDetailScale = hasDetail ? mat.detailScale : 0.0f;
                matProps = MaterialProperties(mat.roughness, mat.metallic, mat.aoStrength, effectiveEmission,
                    effectiveNormal, effectiveSpecular, effectiveDetailScale);
            }

            slot.materialBuffer->SetSubData(&matProps, MaterialProperties::Size(), 0);

            if (slot.binding)
            {
                if (baseColorTex)
                    slot.binding->UpdateTexture(1, baseColorTex, frameIndex);
                if (roughnessTex)
                    slot.binding->UpdateTexture(4, roughnessTex, frameIndex);
                if (metallicTex)
                    slot.binding->UpdateTexture(5, metallicTex, frameIndex);
                if (aoTex)
                    slot.binding->UpdateTexture(6, aoTex, frameIndex);
                if (emissionTex)
                    slot.binding->UpdateTexture(7, emissionTex, frameIndex);

                if (shadowMapTexture)
                    slot.binding->UpdateTexture(8, shadowMapTexture, frameIndex);

                if (normalTex)
                    slot.binding->UpdateTexture(9, normalTex, frameIndex);
                if (specularTex)
                    slot.binding->UpdateTexture(10, specularTex, frameIndex);
                if (detailAlbedoTex)
                    slot.binding->UpdateTexture(11, detailAlbedoTex, frameIndex);
                if (detailNormalTex)
                    slot.binding->UpdateTexture(12, detailNormalTex, frameIndex);

                backend->SubmitDrawCommand(shader.get(), slot.binding.get(), frameIndex, mapbinMesh.mesh->GetUnderlyingMesh());
            }
        }
    }

    void MapGeometryRenderAffector::SetShadowShader(std::unique_ptr<IShaderProgram> s)
    {
        shadowShader = std::move(s);
    }

    void MapGeometryRenderAffector::SetShadowMapTexture(ITextureHandle* texture)
    {
        shadowMapTexture = texture;
    }

    void MapGeometryRenderAffector::RenderShadowDepth(
        Common::Component::ComponentModule& componentModule,
        IRenderBackend* backend,
        const glm::mat4& lightViewProjection)
    {
        if (!backend || !shadowShader || !shadowShader->IsValid() || !bufferManager || !uniformManager)
            return;

        int32_t frameIndex = backend->GetCurrentFrame();
        shadowUniformPoolIndex = 0;

        auto& pool = componentModule.GetPool<Components::MapbinMesh>();

        for (auto [entity, mapbinMesh] : pool)
        {
            if (!mapbinMesh.mesh || !mapbinMesh.mesh->IsValid())
                continue;

            auto transformOpt = componentModule.GetComponent<Common::Component::Transform>(entity);

            if (!transformOpt.has_value())
                continue;

            auto& transform = transformOpt->get();

            StandardUniforms uniforms(transform.worldMatrix, lightViewProjection, glm::vec4(0.0f));

            auto& slot = AcquireShadowUniformSlot();

            slot.buffer->SetSubData(&uniforms, StandardUniforms::Size(), 0);

            if (slot.binding)
                backend->SubmitDrawCommand(shadowShader.get(), slot.binding.get(), frameIndex, mapbinMesh.mesh->GetUnderlyingMesh());
        }
    }

    void MapGeometryRenderAffector::Cleanup()
    {
        sceneMeshes.clear();
        sceneTextures.clear();
        loadedMaterials.clear();
        uniformPool.clear();
        uniformPoolIndex = 0;
        shadowUniformPool.clear();
        shadowUniformPoolIndex = 0;
        processedMapEntities.clear();
        shader.reset();
        shadowShader.reset();
        shadowMapTexture = nullptr;
        bufferManager = nullptr;
        uniformManager = nullptr;
        textureManager = nullptr;
        sceneLightingBuffer = nullptr;
    }

    MapGeometryRenderAffector::UniformSlot& MapGeometryRenderAffector::AcquireUniformSlot()
    {
        if (uniformPoolIndex < uniformPool.size())
            return uniformPool[uniformPoolIndex++];

        UniformSlot slot;
        slot.buffer = bufferManager->CreateUniformBuffer(StandardUniforms::Size());
        slot.materialBuffer = bufferManager->CreateUniformBuffer(MaterialProperties::Size());
        slot.binding = uniformManager->CreateBindingForShader(shader.get());

        if (slot.binding)
        {
            slot.binding->UpdateBuffer(0, slot.buffer.get(), StandardUniforms::Size());

            if (textureManager)
            {
                slot.binding->UpdateTexture(1, textureManager->GetDefaultTexture());
                slot.binding->UpdateTexture(4, textureManager->GetDefaultTexture());
                slot.binding->UpdateTexture(5, textureManager->GetDefaultTexture());
                slot.binding->UpdateTexture(6, textureManager->GetDefaultTexture());
                slot.binding->UpdateTexture(7, textureManager->GetDefaultTexture());
                slot.binding->UpdateTexture(8, textureManager->GetDefaultTexture());
                slot.binding->UpdateTexture(9, textureManager->GetDefaultTexture());
                slot.binding->UpdateTexture(10, textureManager->GetDefaultTexture());
                slot.binding->UpdateTexture(11, textureManager->GetDefaultTexture());
                slot.binding->UpdateTexture(12, textureManager->GetDefaultTexture());
            }

            if (sceneLightingBuffer)
                slot.binding->UpdateBuffer(2, sceneLightingBuffer, Framework::SceneLightingData::Size());

            slot.binding->UpdateBuffer(3, slot.materialBuffer.get(), MaterialProperties::Size());
        }

        uniformPool.push_back(std::move(slot));
        return uniformPool[uniformPoolIndex++];
    }

    MapGeometryRenderAffector::ShadowUniformSlot& MapGeometryRenderAffector::AcquireShadowUniformSlot()
    {
        if (shadowUniformPoolIndex < shadowUniformPool.size())
            return shadowUniformPool[shadowUniformPoolIndex++];

        ShadowUniformSlot slot;
        slot.buffer = bufferManager->CreateUniformBuffer(StandardUniforms::Size());
        slot.binding = uniformManager->CreateBindingForShader(shadowShader.get());

        if (slot.binding)
            slot.binding->UpdateBuffer(0, slot.buffer.get(), StandardUniforms::Size());

        shadowUniformPool.push_back(std::move(slot));
        return shadowUniformPool[shadowUniformPoolIndex++];
    }
}
