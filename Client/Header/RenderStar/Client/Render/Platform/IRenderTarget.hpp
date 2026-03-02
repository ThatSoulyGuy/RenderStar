#pragma once

#include "RenderStar/Client/Render/Resource/ITextureHandle.hpp"
#include <cstdint>
#include <string>

namespace RenderStar::Client::Render::Platform
{
    enum class RenderTargetType
    {
        COLOR_DEPTH,
        COLOR_ONLY,
        DEPTH_ONLY,
        SWAPCHAIN
    };

    struct RenderTargetDescription
    {
        std::string name;
        uint32_t width = 0;
        uint32_t height = 0;
        TextureFormat colorFormat = TextureFormat::RGBA8;
        bool hasDepth = false;
        bool matchSwapchainSize = true;
        uint32_t sampleCount = 1;
    };

    class IRenderTarget
    {
    public:

        virtual ~IRenderTarget() = default;

        virtual const std::string& GetName() const = 0;

        virtual ITextureHandle* GetColorAttachment(uint32_t index) const = 0;

        virtual ITextureHandle* GetDepthAttachment() const = 0;

        virtual uint32_t GetWidth() const = 0;

        virtual uint32_t GetHeight() const = 0;

        virtual void Resize(uint32_t width, uint32_t height) = 0;

        virtual RenderTargetType GetType() const = 0;

        virtual bool IsSwapchain() const = 0;

        virtual uint32_t GetSampleCount() const { return 1; }
    };
}
