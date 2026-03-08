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
#include "RenderStar/Common/Physics/MovementModel.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
        BuildCapsuleMesh(Common::Physics::PlayerDimensions::CAPSULE_RADIUS,
                         Common::Physics::PlayerDimensions::CAPSULE_HEIGHT, 24, 8);
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
        capsuleMesh.reset();
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
        if (!backend || !shader || !shader->IsValid() || !capsuleMesh || !capsuleMesh->IsValid() || !bufferManager || !uniformManager)
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

            // Offset capsule up so feet are at transform.position (mesh is centered at Y=0)
            glm::mat4 modelMatrix = transform.worldMatrix;
            modelMatrix[3][1] += Common::Physics::PlayerDimensions::TOTAL_HEIGHT * 0.5f;

            StandardUniforms uniforms(modelMatrix, viewProjection, glm::vec4(0.2f, 0.6f, 1.0f, 0.0f));

            auto& slot = AcquireUniformSlot();

            slot.buffer->SetSubData(&uniforms, StandardUniforms::Size(), 0);

            MaterialProperties matProps(0.5f, 0.0f, 1.0f, 0.0f, 0.0f);
            slot.materialBuffer->SetSubData(&matProps, MaterialProperties::Size(), 0);

            if (slot.binding)
                backend->SubmitDrawCommand(shader.get(), slot.binding.get(), frameIndex, capsuleMesh->GetUnderlyingMesh());
        }
    }

    void PlayerRenderAffector::BuildCapsuleMesh(float radius, float cylinderHeight, int slices, int rings)
    {
        if (!bufferManager)
            return;

        std::vector<Framework::LitVertex> vertices;
        std::vector<uint32_t> indices;

        float halfCyl = cylinderHeight * 0.5f;
        float totalHeight = cylinderHeight + 2.0f * radius;

        auto addVertex = [&](float px, float py, float pz, float nx, float ny, float nz, float u, float v)
        {
            float tx = -nz;
            float ty = 0.0f;
            float tz = nx;
            float len = std::sqrt(tx * tx + tz * tz);

            if (len > 0.0001f) { tx /= len; tz /= len; }
            else { tx = 1.0f; tz = 0.0f; }

            vertices.push_back({ px, py, pz, nx, ny, nz, u, v, tx, ty, tz });
        };

        // Top hemisphere (center at y = halfCyl)
        for (int ring = 0; ring <= rings; ++ring)
        {
            float phi = static_cast<float>(ring) / static_cast<float>(rings) * static_cast<float>(M_PI) * 0.5f;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            for (int slice = 0; slice <= slices; ++slice)
            {
                float theta = static_cast<float>(slice) / static_cast<float>(slices) * static_cast<float>(M_PI) * 2.0f;
                float sinTheta = std::sin(theta);
                float cosTheta = std::cos(theta);

                float nx = cosTheta * sinPhi;
                float ny = cosPhi;
                float nz = sinTheta * sinPhi;

                float px = radius * nx;
                float py = halfCyl + radius * ny;
                float pz = radius * nz;

                float u = static_cast<float>(slice) / static_cast<float>(slices);
                float v = (py + halfCyl + radius) / totalHeight;

                addVertex(px, py, pz, nx, ny, nz, u, v);
            }
        }

        uint32_t topHemiVerts = static_cast<uint32_t>((rings + 1) * (slices + 1));

        for (int ring = 0; ring < rings; ++ring)
        {
            for (int slice = 0; slice < slices; ++slice)
            {
                uint32_t a = static_cast<uint32_t>(ring * (slices + 1) + slice);
                uint32_t b = a + static_cast<uint32_t>(slices + 1);

                indices.push_back(a);
                indices.push_back(b);
                indices.push_back(a + 1);

                indices.push_back(a + 1);
                indices.push_back(b);
                indices.push_back(b + 1);
            }
        }

        // Cylinder body (2 rings: top at y = halfCyl, bottom at y = -halfCyl)
        uint32_t cylBase = static_cast<uint32_t>(vertices.size());

        for (int slice = 0; slice <= slices; ++slice)
        {
            float theta = static_cast<float>(slice) / static_cast<float>(slices) * static_cast<float>(M_PI) * 2.0f;
            float cosTheta = std::cos(theta);
            float sinTheta = std::sin(theta);

            float u = static_cast<float>(slice) / static_cast<float>(slices);

            addVertex(radius * cosTheta, halfCyl, radius * sinTheta,
                      cosTheta, 0.0f, sinTheta, u, (halfCyl + halfCyl + radius) / totalHeight);

            addVertex(radius * cosTheta, -halfCyl, radius * sinTheta,
                      cosTheta, 0.0f, sinTheta, u, (-halfCyl + halfCyl + radius) / totalHeight);
        }

        for (int slice = 0; slice < slices; ++slice)
        {
            uint32_t top = cylBase + static_cast<uint32_t>(slice * 2);
            uint32_t bot = top + 1;

            indices.push_back(top);
            indices.push_back(bot);
            indices.push_back(top + 2);

            indices.push_back(top + 2);
            indices.push_back(bot);
            indices.push_back(bot + 2);
        }

        // Bottom hemisphere (center at y = -halfCyl)
        uint32_t botBase = static_cast<uint32_t>(vertices.size());

        for (int ring = 0; ring <= rings; ++ring)
        {
            float phi = static_cast<float>(M_PI) * 0.5f + static_cast<float>(ring) / static_cast<float>(rings) * static_cast<float>(M_PI) * 0.5f;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            for (int slice = 0; slice <= slices; ++slice)
            {
                float theta = static_cast<float>(slice) / static_cast<float>(slices) * static_cast<float>(M_PI) * 2.0f;
                float sinTheta = std::sin(theta);
                float cosTheta = std::cos(theta);

                float nx = cosTheta * sinPhi;
                float ny = cosPhi;
                float nz = sinTheta * sinPhi;

                float px = radius * nx;
                float py = -halfCyl + radius * ny;
                float pz = radius * nz;

                float u = static_cast<float>(slice) / static_cast<float>(slices);
                float v = (py + halfCyl + radius) / totalHeight;

                addVertex(px, py, pz, nx, ny, nz, u, v);
            }
        }

        for (int ring = 0; ring < rings; ++ring)
        {
            for (int slice = 0; slice < slices; ++slice)
            {
                uint32_t a = botBase + static_cast<uint32_t>(ring * (slices + 1) + slice);
                uint32_t b = a + static_cast<uint32_t>(slices + 1);

                indices.push_back(a);
                indices.push_back(b);
                indices.push_back(a + 1);

                indices.push_back(a + 1);
                indices.push_back(b);
                indices.push_back(b + 1);
            }
        }

        capsuleMesh = std::make_unique<Resource::Mesh>(*bufferManager, Framework::LitVertex::LAYOUT, PrimitiveType::TRIANGLES);
        capsuleMesh->SetVertices(vertices);
        capsuleMesh->SetIndices(indices);
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
}
