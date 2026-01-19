#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    using BufferHandle = uint32_t;

    class OpenGLBufferManager
    {
    public:

        static constexpr BufferHandle INVALID_BUFFER = 0;

        OpenGLBufferManager();

        ~OpenGLBufferManager();

        BufferHandle CreateVertexBuffer(const void* data, size_t size);

        BufferHandle CreateIndexBuffer(const void* data, size_t size);

        BufferHandle CreateUniformBuffer(size_t size);

        void UpdateBuffer(BufferHandle handle, const void* data, size_t size, size_t offset);

        void DestroyBuffer(BufferHandle handle);

        void BindVertexBuffer(BufferHandle handle);

        void BindIndexBuffer(BufferHandle handle);

        void BindUniformBuffer(BufferHandle handle, uint32_t bindingPoint);

    private:

        std::shared_ptr<spdlog::logger> logger;
        std::vector<BufferHandle> allocatedBuffers;
    };
}
