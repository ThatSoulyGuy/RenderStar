#pragma once

#include <cstdint>
#include <cstddef>

namespace RenderStar::Client::Render
{
    enum class BufferType
    {
        VERTEX,
        INDEX,
        UNIFORM,
        STORAGE
    };

    enum class BufferUsage
    {
        STATIC,
        DYNAMIC,
        STREAM
    };

    class IBufferHandle
    {
    public:

        virtual ~IBufferHandle() = default;

        virtual void SetData(const void* data, size_t size) = 0;
        virtual void SetSubData(const void* data, size_t size, size_t offset) = 0;

        virtual size_t GetSize() const = 0;
        virtual BufferType GetType() const = 0;
        virtual BufferUsage GetUsage() const = 0;

        virtual bool IsValid() const = 0;
    };
}
