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
    class OpenGLTextureManager;
    class OpenGLCommandQueue;

    struct OpenGLDrawCommand
    {
        enum class Type { Draw, SetScissor, ClearScissor };
        Type type = Type::Draw;

        IShaderProgram* shader = nullptr;
        IUniformBindingHandle* uniformBinding = nullptr;
        int32_t frameIndex = 0;
        IMesh* mesh = nullptr;

        int32_t scissorX = 0, scissorY = 0;
        uint32_t scissorW = 0, scissorH = 0;
    };

    class OpenGLRenderBackend : public IRenderBackend
    {
    public:

        static constexpr int32_t MAX_FRAMES_IN_FLIGHT = 2;

        OpenGLRenderBackend();

        ~OpenGLRenderBackend() override;

        [[nodiscard]]
        RenderBackend GetType() const override;

        [[nodiscard]]
        const BackendCapabilities& GetCapabilities() const override;

        void Initialize(GLFWwindow* windowHandle, uint32_t width, uint32_t height) override;

        void Destroy() override;

        void BeginFrame() override;

        void EndFrame() override;

        void WaitIdle() override;

        void OnResize(uint32_t width, uint32_t height) override;

        [[nodiscard]]
        uint32_t GetWidth() const override;

        [[nodiscard]]
        uint32_t GetHeight() const override;

        [[nodiscard]]
        int32_t GetCurrentFrame() const override;

        [[nodiscard]]
        int32_t GetMaxFramesInFlight() const override;

        IBufferManager* GetBufferManager() override;
        IShaderManager* GetShaderManager() override;
        IUniformManager* GetUniformManager() override;
        ITextureManager* GetTextureManager() override;
        IGraphicsResourceManager* GetResourceManager() override;
        IRenderCommandQueue* GetCommandQueue() override;

        void SubmitDrawCommand(IShaderProgram* shader, IUniformBindingHandle* uniformBinding, int32_t frameIndex, IMesh* mesh) override;
        void ExecuteDrawCommands() override;

        void BeginOverlayPass() override;
        void EndOverlayPass() override;

        void SetScissorRect(int32_t x, int32_t y, uint32_t w, uint32_t h) override;
        void ClearScissorRect() override;

        void SubmitSetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h) override;
        void SubmitClearScissor() override;

        [[nodiscard]]
        bool IsInitialized() const override;

    private:

        std::shared_ptr<spdlog::logger> logger;
        GLFWwindow* window;
        uint32_t width;
        uint32_t height;
        int32_t currentFrame;
        bool initialized;
        BackendCapabilities capabilities;
        std::vector<OpenGLDrawCommand> drawCommands;
        std::unique_ptr<OpenGLBufferManagerAdapter> bufferManager;
        std::unique_ptr<OpenGLUniformManagerAdapter> uniformManager;
        std::unique_ptr<OpenGLShaderManagerAdapter> shaderManager;
        std::unique_ptr<OpenGLTextureManager> textureManager;
        std::unique_ptr<OpenGLCommandQueue> commandQueue;
        uint32_t emptyVAO = 0;
    };
}
