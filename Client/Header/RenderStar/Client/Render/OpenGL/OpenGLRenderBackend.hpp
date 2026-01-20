#pragma once

#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include <spdlog/spdlog.h>
#include <vector>
#include <memory>

struct GLFWwindow;

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLBufferManagerAdapter;
    class OpenGLUniformManagerAdapter;
    class OpenGLShaderManagerAdapter;
    class OpenGLCommandQueue;

    struct OpenGLDrawCommand
    {
        IShaderProgram* shader;
        IUniformBindingHandle* uniformBinding;
        int32_t frameIndex;
        IMesh* mesh;
    };

    class OpenGLRenderBackend : public IRenderBackend
    {
    public:

        static constexpr int32_t MAX_FRAMES_IN_FLIGHT = 2;

        OpenGLRenderBackend();

        ~OpenGLRenderBackend() override;

        RenderBackend GetType() const override;
        const BackendCapabilities& GetCapabilities() const override;

        void Initialize(GLFWwindow* windowHandle, int32_t width, int32_t height) override;

        void Destroy() override;

        void BeginFrame() override;

        void EndFrame() override;

        void WaitIdle() override;

        void OnResize(int32_t width, int32_t height) override;

        int32_t GetWidth() const override;

        int32_t GetHeight() const override;

        int32_t GetCurrentFrame() const override;

        int32_t GetMaxFramesInFlight() const override;

        IBufferManager* GetBufferManager() override;
        IShaderManager* GetShaderManager() override;
        IUniformManager* GetUniformManager() override;
        IRenderCommandQueue* GetCommandQueue() override;

        void SubmitDrawCommand(IShaderProgram* shader, IUniformBindingHandle* uniformBinding, int32_t frameIndex, IMesh* mesh) override;
        void ExecuteDrawCommands() override;

        bool IsInitialized() const override;

    private:

        std::shared_ptr<spdlog::logger> logger;
        GLFWwindow* window;
        int32_t width;
        int32_t height;
        int32_t currentFrame;
        bool initialized;
        BackendCapabilities capabilities;
        std::vector<OpenGLDrawCommand> drawCommands;
        std::unique_ptr<OpenGLBufferManagerAdapter> bufferManager;
        std::unique_ptr<OpenGLUniformManagerAdapter> uniformManager;
        std::unique_ptr<OpenGLShaderManagerAdapter> shaderManager;
        std::unique_ptr<OpenGLCommandQueue> commandQueue;
    };
}
