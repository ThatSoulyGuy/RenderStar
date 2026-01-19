#include "RenderStar/Client/Render/OpenGL/OpenGLRenderBackend.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLBufferManagerAdapter.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLUniformManagerAdapter.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLShaderManagerAdapter.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/IMesh.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLRenderBackend::OpenGLRenderBackend()
        : logger(spdlog::default_logger())
        , window(nullptr)
        , width(0)
        , height(0)
        , currentFrame(0)
        , initialized(false)
        , capabilities(BackendCapabilities::ForOpenGL45())
    {
    }

    OpenGLRenderBackend::~OpenGLRenderBackend()
    {
        if (initialized)
            Destroy();
    }

    RenderBackend OpenGLRenderBackend::GetType() const
    {
        return RenderBackend::OPENGL;
    }

    const BackendCapabilities& OpenGLRenderBackend::GetCapabilities() const
    {
        return capabilities;
    }

    void OpenGLRenderBackend::Initialize(GLFWwindow* windowHandle, int32_t initialWidth, int32_t initialHeight)
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

        glViewport(0, 0, width, height);
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

        initialized = true;
        logger->info("OpenGL render backend initialized ({}x{})", width, height);
    }

    void OpenGLRenderBackend::Destroy()
    {
        shaderManager.reset();
        uniformManager.reset();
        bufferManager.reset();

        initialized = false;
        logger->info("OpenGL render backend destroyed");
    }

    void OpenGLRenderBackend::BeginFrame()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
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

    void OpenGLRenderBackend::OnResize(int32_t newWidth, int32_t newHeight)
    {
        width = newWidth;
        height = newHeight;
        glViewport(0, 0, width, height);
    }

    int32_t OpenGLRenderBackend::GetWidth() const
    {
        return width;
    }

    int32_t OpenGLRenderBackend::GetHeight() const
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

    void OpenGLRenderBackend::SubmitDrawCommand(IShaderProgram* shader, IUniformBindingHandle* uniformBinding, int32_t frameIndex, IMesh* mesh)
    {
        drawCommands.push_back(OpenGLDrawCommand{ shader, uniformBinding, frameIndex, mesh });
    }

    void OpenGLRenderBackend::ExecuteDrawCommands()
    {
        for (const auto& command : drawCommands)
        {
            if (command.shader)
                command.shader->Bind();

            if (command.uniformBinding)
                command.uniformBinding->Bind(command.frameIndex);

            if (command.mesh)
            {
                command.mesh->Bind();
                command.mesh->Draw();
                command.mesh->Unbind();
            }

            if (command.shader)
                command.shader->Unbind();
        }

        drawCommands.clear();
    }
}
