#pragma once

#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include "RenderStar/Client/Render/Resource/IMesh.hpp"
#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include <memory>
#include <cstddef>

namespace RenderStar::Client::Render
{
    class IBufferManager
    {
    public:

        virtual ~IBufferManager() = default;

        virtual std::unique_ptr<IBufferHandle> CreateBuffer(
            BufferType type,
            BufferUsage usage,
            size_t size,
            const void* initialData = nullptr) = 0;

        virtual std::unique_ptr<IBufferHandle> CreateVertexBuffer(
            size_t size,
            const void* data,
            BufferUsage usage = BufferUsage::STATIC) = 0;

        virtual std::unique_ptr<IBufferHandle> CreateIndexBuffer(
            size_t size,
            const void* data,
            BufferUsage usage = BufferUsage::STATIC) = 0;

        virtual std::unique_ptr<IBufferHandle> CreateUniformBuffer(
            size_t size,
            BufferUsage usage = BufferUsage::DYNAMIC) = 0;

        virtual std::unique_ptr<IMesh> CreateMesh(
            const VertexLayout& layout,
            PrimitiveType primitive = PrimitiveType::TRIANGLES) = 0;

        virtual void DestroyBuffer(IBufferHandle* buffer) = 0;
    };
}
