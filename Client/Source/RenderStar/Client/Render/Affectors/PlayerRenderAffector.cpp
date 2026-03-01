#include "RenderStar/Client/Render/Affectors/PlayerRenderAffector.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Client/Render/Resource/ITextureManager.hpp"
#include "RenderStar/Client/Render/Resource/IUniformManager.hpp"
#include "RenderStar/Client/Render/Resource/MaterialProperties.hpp"
#include "RenderStar/Client/Render/Resource/StandardUniforms.hpp"
#include "RenderStar/Client/Render/Framework/SceneLightingData.hpp"
#include "RenderStar/Client/Render/Framework/LitVertex.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/PlayerIdentity.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"

namespace RenderStar::Client::Render::Affectors
{
    void PlayerRenderAffector::Affect(Common::Component::ComponentModule&)
    {
    }

    void PlayerRenderAffector::SetupRenderState(IBufferManager* bm, IUniformManager* um, ITextureManager* tm)
    {
        bufferManager = bm;
        uniformManager = um;
        textureManager = tm;
        BuildCubeMesh();
    }

    void PlayerRenderAffector::SetShader(std::unique_ptr<IShaderProgram> s)
    {
        shader = std::move(s);
    }

    void PlayerRenderAffector::SetSceneLightingBuffer(IBufferHandle* buffer)
    {
        sceneLightingBuffer = buffer;
    }

    void PlayerRenderAffector::Cleanup()
    {
        uniformPool.clear();
        cubeMesh.reset();
        uniformPoolIndex = 0;
        shader.reset();
        bufferManager = nullptr;
        uniformManager = nullptr;
        textureManager = nullptr;
        sceneLightingBuffer = nullptr;
    }

    void PlayerRenderAffector::Render(Common::Component::ComponentModule& componentModule, IRenderBackend* backend,
        const glm::mat4& viewProjection, int32_t localPlayerId)
    {
        if (!backend || !shader || !shader->IsValid() || !cubeMesh || !cubeMesh->IsValid() || !bufferManager || !uniformManager)
            return;

        int32_t frameIndex = backend->GetCurrentFrame();
        uniformPoolIndex = 0;

        auto& pool = componentModule.GetPool<Common::Component::PlayerIdentity>();

        for (auto [entity, identity] : pool)
        {
            if (identity.playerId == localPlayerId)
                continue;

            auto transformOpt = componentModule.GetComponent<Common::Component::Transform>(entity);

            if (!transformOpt.has_value())
                continue;

            auto& transform = transformOpt->get();

            StandardUniforms uniforms(transform.worldMatrix, viewProjection, glm::vec4(0.2f, 0.6f, 1.0f, 0.0f));

            auto& slot = AcquireUniformSlot();

            slot.buffer->SetSubData(&uniforms, StandardUniforms::Size(), 0);

            MaterialProperties matProps(0.5f, 0.0f, 1.0f, 0.0f, 0.0f);
            slot.materialBuffer->SetSubData(&matProps, MaterialProperties::Size(), 0);

            if (slot.binding)
                backend->SubmitDrawCommand(shader.get(), slot.binding.get(), frameIndex, cubeMesh->GetUnderlyingMesh());
        }
    }

    void PlayerRenderAffector::BuildCubeMesh()
    {
        if (!bufferManager)
            return;

        const float h = 0.5f;

        std::vector<Framework::LitVertex> vertices =
        {
            { -h, -h,  h,  0, 0, 1,  0, 0,  1, 0, 0 },
            {  h, -h,  h,  0, 0, 1,  1, 0,  1, 0, 0 },
            {  h,  h,  h,  0, 0, 1,  1, 1,  1, 0, 0 },
            { -h,  h,  h,  0, 0, 1,  0, 1,  1, 0, 0 },

            {  h, -h, -h,  0, 0,-1,  0, 0, -1, 0, 0 },
            { -h, -h, -h,  0, 0,-1,  1, 0, -1, 0, 0 },
            { -h,  h, -h,  0, 0,-1,  1, 1, -1, 0, 0 },
            {  h,  h, -h,  0, 0,-1,  0, 1, -1, 0, 0 },

            { -h,  h,  h,  0, 1, 0,  0, 0,  1, 0, 0 },
            {  h,  h,  h,  0, 1, 0,  1, 0,  1, 0, 0 },
            {  h,  h, -h,  0, 1, 0,  1, 1,  1, 0, 0 },
            { -h,  h, -h,  0, 1, 0,  0, 1,  1, 0, 0 },

            { -h, -h, -h,  0,-1, 0,  0, 0,  1, 0, 0 },
            {  h, -h, -h,  0,-1, 0,  1, 0,  1, 0, 0 },
            {  h, -h,  h,  0,-1, 0,  1, 1,  1, 0, 0 },
            { -h, -h,  h,  0,-1, 0,  0, 1,  1, 0, 0 },

            {  h, -h,  h,  1, 0, 0,  0, 0,  0, 0,-1 },
            {  h, -h, -h,  1, 0, 0,  1, 0,  0, 0,-1 },
            {  h,  h, -h,  1, 0, 0,  1, 1,  0, 0,-1 },
            {  h,  h,  h,  1, 0, 0,  0, 1,  0, 0,-1 },

            { -h, -h, -h, -1, 0, 0,  0, 0,  0, 0, 1 },
            { -h, -h,  h, -1, 0, 0,  1, 0,  0, 0, 1 },
            { -h,  h,  h, -1, 0, 0,  1, 1,  0, 0, 1 },
            { -h,  h, -h, -1, 0, 0,  0, 1,  0, 0, 1 },
        };

        std::vector<uint32_t> indices;

        for (uint32_t face = 0; face < 6; ++face)
        {
            uint32_t base = face * 4;
            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }

        cubeMesh = std::make_unique<Resource::Mesh>(*bufferManager, Framework::LitVertex::LAYOUT, PrimitiveType::TRIANGLES);
        cubeMesh->SetVertices(vertices);
        cubeMesh->SetIndices(indices);
    }

    PlayerRenderAffector::UniformSlot& PlayerRenderAffector::AcquireUniformSlot()
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
            }

            if (sceneLightingBuffer)
                slot.binding->UpdateBuffer(2, sceneLightingBuffer, Framework::SceneLightingData::Size());

            slot.binding->UpdateBuffer(3, slot.materialBuffer.get(), MaterialProperties::Size());
        }

        uniformPool.push_back(std::move(slot));
        return uniformPool[uniformPoolIndex++];
    }
}
