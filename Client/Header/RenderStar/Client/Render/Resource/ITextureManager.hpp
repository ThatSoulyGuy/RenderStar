#pragma once

#include "RenderStar/Client/Render/Resource/ITextureHandle.hpp"
#include <memory>

namespace RenderStar::Client::Render
{
    class ITextureManager
    {
    public:

        virtual ~ITextureManager() = default;

        virtual std::unique_ptr<ITextureHandle> CreateFromMemory(
            const TextureDescription& description, const void* pixels) = 0;

        virtual ITextureHandle* GetDefaultTexture() = 0;
    };
}
