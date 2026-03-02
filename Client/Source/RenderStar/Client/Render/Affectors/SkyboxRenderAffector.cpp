#include "RenderStar/Client/Render/Affectors/SkyboxRenderAffector.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Client/Render/Resource/IUniformManager.hpp"
#include "RenderStar/Client/Render/Framework/LitVertex.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"

namespace RenderStar::Client::Render::Affectors
{
    void SkyboxRenderAffector::Affect(Common::Component::ComponentModule&)
    {
    }

    void SkyboxRenderAffector::SetupRenderState(IBufferManager* bm, IUniformManager* um)
    {
        bufferManager = bm;
        uniformManager = um;
        BuildCubeMesh();
    }

    void SkyboxRenderAffector::SetShader(std::unique_ptr<IShaderProgram> s)
    {
        shader = std::move(s);

        if (shader && shader->IsValid() && uniformManager && bufferManager)
        {
            uniformBuffer = bufferManager->CreateUniformBuffer(SkyUniforms::Size());
            binding = uniformManager->CreateBindingForShader(shader.get());

            if (binding && uniformBuffer)
                binding->UpdateBuffer(0, uniformBuffer.get(), SkyUniforms::Size());
        }
    }

    void SkyboxRenderAffector::Cleanup()
    {
        binding.reset();
        uniformBuffer.reset();
        cubeMesh.reset();
        shader.reset();
        bufferManager = nullptr;
        uniformManager = nullptr;
    }

    void SkyboxRenderAffector::Render(IRenderBackend* backend,
        const glm::mat4& viewMatrix,
        const glm::mat4& projectionMatrix,
        const glm::vec3& sunDirection,
        const glm::vec3& sunColor,
        float sunIntensity,
        const glm::vec3& ambientColor,
        float ambientIntensity)
    {
        if (!backend || !shader || !shader->IsValid() || !cubeMesh || !cubeMesh->IsValid() || !binding)
            return;

        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(viewMatrix));

        SkyUniforms uniforms{};
        uniforms.viewProjectionNoTranslation = projectionMatrix * viewNoTranslation;
        uniforms.sunDirection = glm::vec4(glm::normalize(sunDirection), 0.0f);
        uniforms.sunColor = glm::vec4(sunColor, sunIntensity);
        uniforms.ambientColor = glm::vec4(ambientColor, ambientIntensity);
        uniforms.skyParams = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

        uniformBuffer->SetSubData(&uniforms, SkyUniforms::Size(), 0);

        int32_t frameIndex = backend->GetCurrentFrame();
        backend->SubmitDrawCommand(shader.get(), binding.get(), frameIndex, cubeMesh->GetUnderlyingMesh());
    }

    void SkyboxRenderAffector::BuildCubeMesh()
    {
        if (!bufferManager)
            return;

        std::vector<Framework::LitVertex> vertices =
        {
            { -1, -1,  1,  0, 0, 1,  0, 0,  1, 0, 0 },
            {  1, -1,  1,  0, 0, 1,  1, 0,  1, 0, 0 },
            {  1,  1,  1,  0, 0, 1,  1, 1,  1, 0, 0 },
            { -1,  1,  1,  0, 0, 1,  0, 1,  1, 0, 0 },

            {  1, -1, -1,  0, 0,-1,  0, 0, -1, 0, 0 },
            { -1, -1, -1,  0, 0,-1,  1, 0, -1, 0, 0 },
            { -1,  1, -1,  0, 0,-1,  1, 1, -1, 0, 0 },
            {  1,  1, -1,  0, 0,-1,  0, 1, -1, 0, 0 },

            { -1,  1,  1,  0, 1, 0,  0, 0,  1, 0, 0 },
            {  1,  1,  1,  0, 1, 0,  1, 0,  1, 0, 0 },
            {  1,  1, -1,  0, 1, 0,  1, 1,  1, 0, 0 },
            { -1,  1, -1,  0, 1, 0,  0, 1,  1, 0, 0 },

            { -1, -1, -1,  0,-1, 0,  0, 0,  1, 0, 0 },
            {  1, -1, -1,  0,-1, 0,  1, 0,  1, 0, 0 },
            {  1, -1,  1,  0,-1, 0,  1, 1,  1, 0, 0 },
            { -1, -1,  1,  0,-1, 0,  0, 1,  1, 0, 0 },

            {  1, -1,  1,  1, 0, 0,  0, 0,  0, 0,-1 },
            {  1, -1, -1,  1, 0, 0,  1, 0,  0, 0,-1 },
            {  1,  1, -1,  1, 0, 0,  1, 1,  0, 0,-1 },
            {  1,  1,  1,  1, 0, 0,  0, 1,  0, 0,-1 },

            { -1, -1, -1, -1, 0, 0,  0, 0,  0, 0, 1 },
            { -1, -1,  1, -1, 0, 0,  1, 0,  0, 0, 1 },
            { -1,  1,  1, -1, 0, 0,  1, 1,  0, 0, 1 },
            { -1,  1, -1, -1, 0, 0,  0, 1,  0, 0, 1 },
        };

        std::vector<uint32_t> indices;

        for (uint32_t face = 0; face < 6; ++face)
        {
            uint32_t base = face * 4;
            indices.push_back(base + 2);
            indices.push_back(base + 1);
            indices.push_back(base);
            indices.push_back(base + 3);
            indices.push_back(base + 2);
            indices.push_back(base);
        }

        cubeMesh = std::make_unique<Resource::Mesh>(*bufferManager, Framework::LitVertex::LAYOUT, PrimitiveType::TRIANGLES);
        cubeMesh->SetVertices(vertices);
        cubeMesh->SetIndices(indices);
    }
}
