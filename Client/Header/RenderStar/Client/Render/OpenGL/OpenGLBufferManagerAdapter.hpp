#pragma once

#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLBufferManagerAdapter : public IBufferManager
    {
    public:

        OpenGLBufferManagerAdapter();
        ~OpenGLBufferManagerAdapter() override;

        std::unique_ptr<IBufferHandle> CreateBuffer(
            BufferType type,
            BufferUsage usage,
            size_t size,
            const void* initialData = nullptr) override;

        std::unique_ptr<IBufferHandle> CreateVertexBuffer(
            size_t size,
            const void* data,
            BufferUsage usage = BufferUsage::STATIC) override;

        std::unique_ptr<IBufferHandle> CreateIndexBuffer(
            size_t size,
            const void* data,
            BufferUsage usage = BufferUsage::STATIC) override;

        std::unique_ptr<IBufferHandle> CreateUniformBuffer(
            size_t size,
            BufferUsage usage = BufferUsage::DYNAMIC) override;

        std::unique_ptr<IMesh> CreateMesh(
            const VertexLayout& layout,
            PrimitiveType primitive = PrimitiveType::TRIANGLES) override;

        void DestroyBuffer(IBufferHandle* buffer) override;

    private:

        uint32_t ToGLUsage(BufferUsage usage) const;

        std::shared_ptr<spdlog::logger> logger;
    };
}
