#pragma once

#include <cstdint>
#include <cstddef>

namespace RenderStar::Client::Render
{
    class IBufferHandle;

    class IUniformBindingHandle
    {
    public:

        virtual ~IUniformBindingHandle() = default;

        virtual void Bind(int32_t frameIndex) = 0;
        virtual void UpdateBuffer(int32_t binding, IBufferHandle* buffer, size_t size) = 0;
        virtual uint64_t GetNativeHandle(int32_t frameIndex) const = 0;
        virtual void Destroy() = 0;
        virtual bool IsDestroyed() const = 0;
    };
}
