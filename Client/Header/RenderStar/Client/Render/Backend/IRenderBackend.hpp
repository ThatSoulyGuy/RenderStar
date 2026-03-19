#pragma once

#include "RenderStar/Client/Render/Backend/RenderBackend.hpp"
#include "RenderStar/Client/Render/Backend/BackendCapabilities.hpp"
#include <cstdint>

struct GLFWwindow;

namespace RenderStar::Client::Render
{
    class IBufferManager;
    class IShaderManager;
    class IUniformManager;
    class ITextureManager;
    class IGraphicsResourceManager;
    class IShaderProgram;
    class IUniformBindingHandle;
    class IMesh;
    class IRenderCommandQueue;

    class IRenderBackend
    {
    public:

        virtual ~IRenderBackend() = default;

        [[nodiscard]]
        virtual RenderBackend GetType() const = 0;

        [[nodiscard]]
        virtual const BackendCapabilities& GetCapabilities() const = 0;

        virtual void Initialize(GLFWwindow* windowHandle, uint32_t width, uint32_t height) = 0;

        virtual void Destroy() = 0;

        virtual void BeginFrame() = 0;

        virtual void EndFrame() = 0;

        virtual void WaitIdle() = 0;

        virtual void OnResize(uint32_t width, uint32_t height) = 0;

        [[nodiscard]]
        virtual uint32_t GetWidth() const = 0;

        [[nodiscard]]
        virtual uint32_t GetHeight() const = 0;

        [[nodiscard]]
        virtual int32_t GetCurrentFrame() const = 0;

        [[nodiscard]]
        virtual int32_t GetMaxFramesInFlight() const = 0;

        virtual IBufferManager* GetBufferManager() = 0;
        virtual IShaderManager* GetShaderManager() = 0;
        virtual IUniformManager* GetUniformManager() = 0;
        virtual ITextureManager* GetTextureManager() = 0;
        virtual IGraphicsResourceManager* GetResourceManager() = 0;
        virtual IRenderCommandQueue* GetCommandQueue() = 0;

        virtual void SubmitDrawCommand(IShaderProgram* shader, IUniformBindingHandle* uniformBinding, int32_t frameIndex, IMesh* mesh) = 0;
        virtual void ExecuteDrawCommands() = 0;

        virtual void BeginOverlayPass() = 0;
        virtual void EndOverlayPass() = 0;

        virtual void SetScissorRect(int32_t x, int32_t y, uint32_t w, uint32_t h) = 0;
        virtual void ClearScissorRect() = 0;

        virtual void SubmitSetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h) = 0;
        virtual void SubmitClearScissor() = 0;

        [[nodiscard]]
        virtual bool IsInitialized() const = 0;
    };
}
