#pragma once

#include "RenderStar/Client/Render/Platform/IRenderTarget.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IShaderManager.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/IMesh.hpp"
#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include "RenderStar/Client/Render/Shader/RsslTypes.hpp"
#include <memory>
#include <vector>

namespace RenderStar::Client::Render
{
    class IRenderBackend;
    class IBufferManager;
    class IUniformManager;
    class ITextureManager;
    class IRenderCommandQueue;
}

namespace RenderStar::Client::Render::Platform
{
    class IRenderingPlatformBackend
    {
    public:

        virtual ~IRenderingPlatformBackend() = default;

        virtual Shader::RsslTarget GetRsslTarget() const = 0;

        virtual std::unique_ptr<IRenderTarget> CreateRenderTarget(
            const RenderTargetDescription& description) = 0;

        virtual std::unique_ptr<IRenderTarget> CreateSwapchainTarget() = 0;

        virtual std::unique_ptr<IShaderProgram> CompileShader(
            const std::string& vertexGlsl,
            const std::string& fragmentGlsl) = 0;

        virtual std::unique_ptr<IShaderProgram> CompileShaderForTarget(
            const std::string& vertexGlsl,
            const std::string& fragmentGlsl,
            IRenderTarget* target,
            const VertexLayout& vertexLayout)
        {
            return CompileShader(vertexGlsl, fragmentGlsl);
        }

        virtual std::unique_ptr<IShaderProgram> CompileComputeShader(
            const std::string& computeGlsl) = 0;

        virtual void BindInputTextures(
            const std::vector<IRenderTarget*>& inputs,
            IShaderProgram* shader,
            int32_t frameIndex) {}

        virtual void BeginRenderTarget(IRenderTarget* target, bool clear) = 0;

        virtual void EndRenderTarget(IRenderTarget* target) = 0;

        virtual void BlitToScreen(IRenderTarget* source) = 0;

        virtual IBufferManager* GetBufferManager() = 0;

        virtual IUniformManager* GetUniformManager() = 0;

        virtual ITextureManager* GetTextureManager() = 0;

        virtual IRenderCommandQueue* GetCommandQueue() = 0;

        virtual void SubmitDrawCommand(
            IShaderProgram* shader,
            IUniformBindingHandle* uniformBinding,
            int32_t frameIndex,
            IMesh* mesh) = 0;

        virtual void ExecuteDrawCommands() = 0;

        virtual void OnResize(uint32_t width, uint32_t height) = 0;

        virtual uint32_t GetWidth() const = 0;

        virtual uint32_t GetHeight() const = 0;

        virtual int32_t GetCurrentFrame() const = 0;

        virtual int32_t GetMaxFramesInFlight() const = 0;
    };
}
