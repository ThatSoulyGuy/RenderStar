#include "RenderStar/Client/Render/OpenGL/OpenGLRenderBackend.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLBufferManagerAdapter.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLUniformManagerAdapter.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLShaderManagerAdapter.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLCommandQueue.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLShaderProgram.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLMeshAdapter.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/IMesh.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLRenderBackend::OpenGLRenderBackend() : logger(spdlog::default_logger()), window(nullptr), width(0), height(0), currentFrame(0), initialized(false), capabilities(BackendCapabilities::ForOpenGL45()) { }

    OpenGLRenderBackend::~OpenGLRenderBackend()
    {
        if (initialized)
            OpenGLRenderBackend::Destroy();
    }

    RenderBackend OpenGLRenderBackend::GetType() const
    {
        return RenderBackend::OPENGL;
    }

    const BackendCapabilities& OpenGLRenderBackend::GetCapabilities() const
    {
        return capabilities;
    }

    void OpenGLRenderBackend::Initialize(GLFWwindow* windowHandle, uint32_t initialWidth, uint32_t initialHeight)
    {
        window = windowHandle;
        width = initialWidth;
        height = initialHeight;

        if (window == nullptr)
        {
            logger->error("Window handle is null");
            return;
        }

        glfwMakeContextCurrent(window);

        int version = gladLoadGL(glfwGetProcAddress);

        if (version == 0)
        {
            logger->error("Failed to initialize GLAD");
            return;
        }

        int major = GLAD_VERSION_MAJOR(version);
        int minor = GLAD_VERSION_MINOR(version);

        logger->info("OpenGL {}.{} loaded via GLAD", major, minor);

        if (major < 4 || (major == 4 && minor < 6))
            logger->warn("OpenGL 4.6 requested but only {}.{} available", major, minor);

        const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        const char* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

        logger->info("OpenGL Vendor: {}", vendor ? vendor : "unknown");
        logger->info("OpenGL Renderer: {}", renderer ? renderer : "unknown");
        logger->info("OpenGL Version: {}", glVersion ? glVersion : "unknown");

        glfwSwapInterval(1);

        glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
        glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        bufferManager = std::make_unique<OpenGLBufferManagerAdapter>();
        uniformManager = std::make_unique<OpenGLUniformManagerAdapter>();
        shaderManager = std::make_unique<OpenGLShaderManagerAdapter>();
        commandQueue = std::make_unique<OpenGLCommandQueue>();

        initialized = true;
        logger->info("OpenGL render backend initialized ({}x{})", width, height);
    }

    void OpenGLRenderBackend::Destroy()
    {
        commandQueue.reset();
        shaderManager.reset();
        uniformManager.reset();
        bufferManager.reset();

        initialized = false;
        logger->info("OpenGL render backend destroyed");
    }

    void OpenGLRenderBackend::BeginFrame()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
        glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRenderBackend::EndFrame()
    {
        glfwSwapBuffers(window);
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void OpenGLRenderBackend::WaitIdle()
    {
        glFinish();
    }

    void OpenGLRenderBackend::OnResize(const uint32_t width, const uint32_t height)
    {
        this->width = width;
        this->height = height;

        glViewport(0, 0, static_cast<int>(this->width), static_cast<int>(this->height));
    }

    uint32_t OpenGLRenderBackend::GetWidth() const
    {
        return width;
    }

    uint32_t OpenGLRenderBackend::GetHeight() const
    {
        return height;
    }

    int32_t OpenGLRenderBackend::GetCurrentFrame() const
    {
        return currentFrame;
    }

    int32_t OpenGLRenderBackend::GetMaxFramesInFlight() const
    {
        return MAX_FRAMES_IN_FLIGHT;
    }

    bool OpenGLRenderBackend::IsInitialized() const
    {
        return initialized;
    }

    IBufferManager* OpenGLRenderBackend::GetBufferManager()
    {
        return bufferManager.get();
    }

    IShaderManager* OpenGLRenderBackend::GetShaderManager()
    {
        return shaderManager.get();
    }

    IUniformManager* OpenGLRenderBackend::GetUniformManager()
    {
        return uniformManager.get();
    }

    IRenderCommandQueue* OpenGLRenderBackend::GetCommandQueue()
    {
        return commandQueue.get();
    }

    void OpenGLRenderBackend::SubmitDrawCommand(IShaderProgram* shader, IUniformBindingHandle* uniformBinding, const int32_t frameIndex, IMesh* mesh)
    {
        drawCommands.push_back(OpenGLDrawCommand{ shader, uniformBinding, frameIndex, mesh });
    }

    void OpenGLRenderBackend::ExecuteDrawCommands()
    {
        for (const auto& [shader, uniformBinding, frameIndex, mesh] : drawCommands)
        {
            auto* glShader = dynamic_cast<OpenGLShaderProgram*>(shader);
            auto* glMesh = dynamic_cast<OpenGLMeshAdapter*>(mesh);

            if (glShader)
                glShader->Bind();

            if (uniformBinding)
                uniformBinding->Bind(frameIndex);

            if (glMesh && glMesh->IsValid())
            {
                glMesh->Bind();
                glMesh->Draw();
                glMesh->Unbind();
            }

            if (glShader)
                glShader->Unbind();
        }

        drawCommands.clear();
    }
}
