#pragma once

#include "RenderStar/Client/Render/Platform/IRenderTarget.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLTextureHandle.hpp"
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLRenderTarget : public Platform::IRenderTarget
    {
    public:

        OpenGLRenderTarget(const Platform::RenderTargetDescription& description);
        ~OpenGLRenderTarget() override;

        const std::string& GetName() const override;
        ITextureHandle* GetColorAttachment(uint32_t index) const override;
        ITextureHandle* GetDepthAttachment() const override;
        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;
        void Resize(uint32_t width, uint32_t height) override;
        Platform::RenderTargetType GetType() const override;
        bool IsSwapchain() const override;

        uint32_t GetFramebufferHandle() const;
        uint32_t GetSampleCount() const override;
        void Bind() const;
        void Unbind() const;

    private:

        void Create();
        void Destroy();

        std::shared_ptr<spdlog::logger> logger;
        Platform::RenderTargetDescription description;
        uint32_t fbo = 0;
        uint32_t msaaFbo = 0;
        uint32_t msaaColorRbo = 0;
        uint32_t msaaDepthRbo = 0;
        std::unique_ptr<OpenGLTextureHandle> colorAttachment;
        std::unique_ptr<OpenGLTextureHandle> depthAttachment;
    };

    class OpenGLSwapchainTarget : public Platform::IRenderTarget
    {
    public:

        OpenGLSwapchainTarget(uint32_t width, uint32_t height);

        const std::string& GetName() const override;
        ITextureHandle* GetColorAttachment(uint32_t index) const override;
        ITextureHandle* GetDepthAttachment() const override;
        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;
        void Resize(uint32_t width, uint32_t height) override;
        Platform::RenderTargetType GetType() const override;
        bool IsSwapchain() const override;

    private:

        std::string name;
        uint32_t width;
        uint32_t height;
    };
}
