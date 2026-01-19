#include "RenderStar/Client/Render/RendererModule.hpp"
#include "RenderStar/Client/Render/Backend/BackendFactory.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Resource/IShaderManager.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include "RenderStar/Client/Render/Resource/IUniformManager.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/IMesh.hpp"
#include "RenderStar/Client/Render/Resource/Vertex.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLShaderProgram.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLMesh.hpp"
#include "RenderStar/Client/Render/Components/Camera.hpp"
#include "RenderStar/Client/Core/ClientWindowModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Configuration/ConfigurationFactory.hpp"
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>

namespace RenderStar::Client::Render
{
    namespace
    {
        const char* TEST_VERTEX_SHADER = R"(
#version 460 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

uniform mat4 uModel;
uniform mat4 uViewProjection;

out vec3 vColor;

void main()
{
    gl_Position = uViewProjection * uModel * vec4(aPosition, 1.0);
    vColor = aColor;
}
)";

        const char* TEST_FRAGMENT_SHADER = R"(
#version 460 core
in vec3 vColor;
out vec4 fragColor;

uniform vec4 uColorTint;

void main()
{
    vec3 finalColor = mix(vColor, uColorTint.rgb, uColorTint.a);
    fragColor = vec4(finalColor, 1.0);
}
)";
    }

    RendererModule::RendererModule()
        : backend(nullptr)
        , backendType(RenderBackend::OPENGL)
        , openglTestShader(nullptr)
        , openglTestMesh(nullptr)
        , testVertexBuffer(0)
        , testIndexBuffer(0)
        , vulkanTestShader(nullptr)
        , vulkanTestMesh(nullptr)
        , vulkanUniformBinding(nullptr)
        , vulkanUniformBuffer(nullptr)
        , testGeometryInitialized(false)
        , rotationAngle(0.0f)
        , aspectRatio(16.0f / 9.0f)
    {
    }

    RendererModule::~RendererModule()
    {
        if (backend != nullptr)
            backend->WaitIdle();

        vulkanUniformBinding.reset();
        vulkanUniformBuffer.reset();
        vulkanTestMesh.reset();
        vulkanTestShader.reset();

        if (testVertexBuffer != 0)
            glDeleteBuffers(1, &testVertexBuffer);

        if (testIndexBuffer != 0)
            glDeleteBuffers(1, &testIndexBuffer);

        openglTestMesh.reset();
        openglTestShader.reset();

        if (backend != nullptr)
            backend->Destroy();
    }

    void RendererModule::RenderFrame()
    {
        if (backend == nullptr || !backend->IsInitialized())
            return;

        backend->BeginFrame();

        if (testGeometryInitialized)
        {
            if (backendType == RenderBackend::OPENGL)
                RenderOpenGLTestGeometry();
            else
                RenderVulkanTestGeometry();
        }

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

        if (backendType == RenderBackend::OPENGL)
            InitializeOpenGLTestGeometry();
        else
            InitializeVulkanTestGeometry();

        logger->info("RendererModule initialized with {} backend",
            backendType == RenderBackend::OPENGL ? "OpenGL" : "Vulkan");
    }

    void RendererModule::InitializeOpenGLTestGeometry()
    {
        openglTestShader = std::make_unique<OpenGL::OpenGLShaderProgram>();

        if (!openglTestShader->CompileFromSource(TEST_VERTEX_SHADER, TEST_FRAGMENT_SHADER))
        {
            logger->error("Failed to compile test shader");
            return;
        }

        float vertices[] = {
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f
        };

        uint32_t indices[] = {
            0, 2, 1, 0, 3, 2,
            4, 5, 6, 4, 6, 7,
            0, 7, 4, 0, 3, 7,
            1, 2, 6, 1, 6, 5,
            3, 6, 2, 3, 7, 6,
            0, 1, 5, 0, 5, 4
        };

        glGenBuffers(1, &testVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, testVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &testIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, testIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        openglTestMesh = std::make_unique<OpenGL::OpenGLMesh>();

        std::vector<OpenGL::VertexAttribute> attributes = {
            { 0, 3, GL_FLOAT, false, 6 * sizeof(float), 0 },
            { 1, 3, GL_FLOAT, false, 6 * sizeof(float), 3 * sizeof(float) }
        };

        openglTestMesh->Create(attributes);
        openglTestMesh->SetVertexBuffer(testVertexBuffer);
        openglTestMesh->SetIndexBuffer(testIndexBuffer);

        aspectRatio = static_cast<float>(backend->GetWidth()) / static_cast<float>(backend->GetHeight());

        testGeometryInitialized = true;
        logger->info("OpenGL test geometry initialized successfully");
    }

    void RendererModule::RenderOpenGLTestGeometry()
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

        openglTestShader->Bind();

        GLint modelLoc = glGetUniformLocation(openglTestShader->GetHandle(), "uModel");
        GLint vpLoc = glGetUniformLocation(openglTestShader->GetHandle(), "uViewProjection");
        GLint tintLoc = glGetUniformLocation(openglTestShader->GetHandle(), "uColorTint");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(viewProjection));

        glm::vec4 colorTint = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
        glUniform4fv(tintLoc, 1, glm::value_ptr(colorTint));

        openglTestMesh->Draw(36);
        openglTestShader->Unbind();
    }

    void RendererModule::InitializeVulkanTestGeometry()
    {
        IShaderManager* shaderManager = backend->GetShaderManager();
        IBufferManager* bufferManager = backend->GetBufferManager();
        IUniformManager* uniformManager = backend->GetUniformManager();

        if (!shaderManager || !bufferManager || !uniformManager)
        {
            logger->error("Failed to get Vulkan managers");
            return;
        }

        std::filesystem::path basePath = Common::Configuration::ConfigurationFactory::GetBasePath();
        std::filesystem::path shaderDir = basePath / "assets" / "renderstar" / "shader";
        std::string vertexPath = (shaderDir / "test.vert").string();
        std::string fragmentPath = (shaderDir / "test.frag").string();

        vulkanTestShader = shaderManager->LoadFromFile(vertexPath, fragmentPath);

        if (!vulkanTestShader || !vulkanTestShader->IsValid())
        {
            logger->error("Failed to load Vulkan test shader from {} and {}", vertexPath, fragmentPath);
            return;
        }

        Vertex vertices[] = {
            Vertex(-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
            Vertex( 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f),
            Vertex( 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
            Vertex(-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f),
            Vertex(-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f),
            Vertex( 0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f),
            Vertex( 0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f),
            Vertex(-0.5f,  0.5f,  0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 1.0f)
        };

        uint32_t indices[] = {
            0, 2, 1, 0, 3, 2,
            4, 5, 6, 4, 6, 7,
            0, 7, 4, 0, 3, 7,
            1, 2, 6, 1, 6, 5,
            3, 6, 2, 3, 7, 6,
            0, 1, 5, 0, 5, 4
        };

        std::vector<float> vertexData = Vertex::ToFloatArray(std::span(vertices));

        vulkanTestMesh = bufferManager->CreateMesh(Vertex::LAYOUT, PrimitiveType::TRIANGLES);
        vulkanTestMesh->SetVertexData(vertexData.data(), vertexData.size() * sizeof(float));
        vulkanTestMesh->SetIndexData(indices, sizeof(indices), IndexType::UINT32);

        size_t uniformSize = sizeof(glm::mat4) * 2 + sizeof(glm::vec4);
        vulkanUniformBuffer = bufferManager->CreateUniformBuffer(uniformSize);

        vulkanUniformBinding = uniformManager->CreateBindingForShader(vulkanTestShader.get());

        if (vulkanUniformBinding)
            vulkanUniformBinding->UpdateBuffer(0, vulkanUniformBuffer.get(), uniformSize);

        aspectRatio = static_cast<float>(backend->GetWidth()) / static_cast<float>(backend->GetHeight());

        testGeometryInitialized = true;
        logger->info("Vulkan test geometry initialized successfully");
    }

    void RendererModule::RenderVulkanTestGeometry()
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

        struct UniformData {
            glm::mat4 model;
            glm::mat4 viewProjection;
            glm::vec4 colorTint;
        } uniformData;

        uniformData.model = model;
        uniformData.viewProjection = viewProjection;
        uniformData.colorTint = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

        vulkanUniformBuffer->SetSubData(&uniformData, sizeof(uniformData), 0);

        int32_t frameIndex = backend->GetCurrentFrame();
        backend->SubmitDrawCommand(vulkanTestShader.get(), vulkanUniformBinding.get(), frameIndex, vulkanTestMesh.get());
        backend->ExecuteDrawCommands();
    }
}
