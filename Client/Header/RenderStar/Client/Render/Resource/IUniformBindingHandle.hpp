#pragma once

#include "RenderStar/Client/Render/Resource/IGraphicsResource.hpp"
#include <cstdint>
#include <cstddef>

namespace RenderStar::Client::Render
{
    class IBufferHandle;
    class ITextureHandle;

    class IUniformBindingHandle : public IGraphicsResource
    {
    public:

        virtual void Bind(int32_t frameIndex) = 0;
        virtual void UpdateBuffer(int32_t binding, IBufferHandle* buffer, size_t size, int32_t frameIndex = -1) = 0;
        virtual void UpdateTexture(int32_t binding, ITextureHandle* texture, int32_t frameIndex = -1) = 0;
        virtual uint64_t GetNativeHandle(int32_t frameIndex) const = 0;
        virtual void Destroy() = 0;
        virtual bool IsDestroyed() const = 0;
    };
}
