#include "RenderStar/Client/Render/RendererModule.hpp"

#include "RenderStar/Client/Core/ClientWindowModule.hpp"
#include "RenderStar/Client/Render/Backend/BackendFactory.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Components/Camera.hpp"
#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Client/Render/Resource/IMesh.hpp"
#include "RenderStar/Client/Render/Resource/IShaderManager.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/IUniformManager.hpp"
#include "RenderStar/Client/Render/Resource/Mesh.hpp"
#include "RenderStar/Client/Render/Resource/StandardUniforms.hpp"
#include "RenderStar/Client/Render/Resource/Vertex.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Configuration/ConfigurationFactory.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <cstring>
#include <vector>

namespace RenderStar::Client::Render
{
    namespace
    {
        const char* TEST_VERTEX_SHADER = R"(
#version 460 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

layout(std140, binding = 0) uniform StandardUniforms
{
    mat4 model;
    mat4 viewProjection;
    vec4 colorTint;
};

out vec3 vColor;

void main()
{
    gl_Position = viewProjection * model * vec4(aPosition, 1.0);
    vColor = aColor;
}
)";

        const char* TEST_FRAGMENT_SHADER = R"(
#version 460 core
in vec3 vColor;
out vec4 fragColor;

layout(std140, binding = 0) uniform StandardUniforms
{
    mat4 model;
    mat4 viewProjection;
    vec4 colorTint;
};

void main()
{
    vec3 finalColor = mix(vColor, colorTint.rgb, colorTint.a);
    fragColor = vec4(finalColor, 1.0);
}
)";
    }

    RendererModule::RendererModule()
        : backend(nullptr)
        , backendType(RenderBackend::OPENGL)
        , testShader(nullptr)
        , testMesh(nullptr)
        , uniformBinding(nullptr)
        , uniformBuffer(nullptr)
        , testGeometryInitialized(false)
        , rotationAngle(0.0f)
        , aspectRatio(16.0f / 9.0f)
    {
    }

    RendererModule::~RendererModule()
    {
        if (backend != nullptr)
            backend->WaitIdle();

        uniformBinding.reset();
        uniformBuffer.reset();
        testShader.reset();
        testMesh.reset();

        if (backend != nullptr)
            backend->Destroy();
    }

    void RendererModule::RenderFrame()
    {
        if (backend == nullptr || !backend->IsInitialized())
            return;

        backend->BeginFrame();

        if (testGeometryInitialized)
            RenderTestGeometry();

        backend->EndFrame();
    }

    IRenderBackend* RendererModule::GetBackend() const
    {
        return backend.get();
    }

    RenderBackend RendererModule::GetBackendType() const
    {
        return backendType;
    }

    void RendererModule::OnInitialize(Common::Module::ModuleContext& moduleContext)
    {
        auto windowModule = moduleContext.GetModule<Core::ClientWindowModule>();

        if (!windowModule.has_value())
        {
            logger->error("ClientWindowModule not found");
            return;
        }

        backendType = windowModule->get().GetPreferredBackend();
        backend = BackendFactory::Create(backendType);

        if (backend == nullptr)
        {
            logger->error("Failed to create render backend");
            return;
        }

        backend->Initialize(
            windowModule->get().GetWindowHandle(),
            windowModule->get().GetFramebufferWidth(),
            windowModule->get().GetFramebufferHeight()
        );

        InitializeTestGeometry(moduleContext);

        logger->info("RendererModule initialized with {} backend",
            backendType == RenderBackend::OPENGL ? "OpenGL" : "Vulkan");
    }

    void RendererModule::InitializeTestGeometry(Common::Module::ModuleContext& context)
    {
        IShaderManager* shaderManager = backend->GetShaderManager();
        IBufferManager* bufferManager = backend->GetBufferManager();
        IUniformManager* uniformManager = backend->GetUniformManager();

        if (!shaderManager || !bufferManager || !uniformManager)
        {
            logger->error("Failed to get managers from backend");
            return;
        }

        if (backendType == RenderBackend::OPENGL)
        {
            ShaderSource source;
            source.vertexSource = TEST_VERTEX_SHADER;
            source.fragmentSource = TEST_FRAGMENT_SHADER;

            testShader = shaderManager->CreateFromSource(source);

            if (!testShader || !testShader->IsValid())
            {
                logger->error("Failed to compile OpenGL test shader");
                return;
            }
        }
        else
        {
            auto assetModule = context.GetModule<Common::Asset::AssetModule>();

            if (!assetModule.has_value())
            {
                logger->error("AssetModule not found");
                return;
            }

            testShader = shaderManager->LoadFromFile(
                assetModule.value().get(),
                Common::Asset::AssetLocation::Parse("renderstar:shader/test.vert"),
                Common::Asset::AssetLocation::Parse("renderstar:shader/test.frag"));

            if (!testShader || !testShader->IsValid())
            {
                logger->error("Failed to load Vulkan test shader");
                return;
            }
        }

        std::vector<Vertex> vertices = {
            Vertex(-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
            Vertex( 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f),
            Vertex( 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
            Vertex(-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f),
            Vertex(-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f),
            Vertex( 0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f),
            Vertex( 0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f),
            Vertex(-0.5f,  0.5f,  0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 1.0f)
        };

        std::vector<uint32_t> indices = {
            0, 2, 1, 0, 3, 2,
            4, 5, 6, 4, 6, 7,
            0, 7, 4, 0, 3, 7,
            1, 2, 6, 1, 6, 5,
            3, 6, 2, 3, 7, 6,
            0, 1, 5, 0, 5, 4
        };

        testMesh = std::make_unique<Resource::Mesh>(*bufferManager, Vertex::LAYOUT, PrimitiveType::TRIANGLES);
        testMesh->SetVertices(vertices);
        testMesh->SetIndices(indices);

        uniformBuffer = bufferManager->CreateUniformBuffer(StandardUniforms::Size());
        uniformBinding = uniformManager->CreateBindingForShader(testShader.get());

        if (uniformBinding)
            uniformBinding->UpdateBuffer(0, uniformBuffer.get(), StandardUniforms::Size());

        aspectRatio = static_cast<float>(backend->GetWidth()) / static_cast<float>(backend->GetHeight());

        testGeometryInitialized = true;
        logger->info("Test geometry initialized successfully");
    }

    void RendererModule::RenderTestGeometry()
    {
        rotationAngle += 0.5f;
        if (rotationAngle >= 360.0f)
            rotationAngle -= 360.0f;

        Components::Camera camera = Components::Camera::CreatePerspective(60.0f, aspectRatio, 0.1f, 100.0f);

        glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        camera.viewMatrix = glm::lookAt(cameraPosition, cameraTarget, cameraUp);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotationAngle * 0.5f), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 viewProjection = camera.GetViewProjectionMatrix();

        StandardUniforms uniformData(model, viewProjection, glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));

        uniformBuffer->SetSubData(&uniformData, StandardUniforms::Size(), 0);

        int32_t frameIndex = backend->GetCurrentFrame();
        backend->SubmitDrawCommand(testShader.get(), uniformBinding.get(), frameIndex, testMesh->GetUnderlyingMesh());
        backend->ExecuteDrawCommands();
    }
}
