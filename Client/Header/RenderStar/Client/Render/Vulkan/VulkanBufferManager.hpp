#pragma once

#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanBufferModule.hpp"
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanBufferManager : public IBufferManager
    {
    public:

        VulkanBufferManager();
        ~VulkanBufferManager() override;

        void Initialize(VulkanBufferModule* bufferModule);

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

        VulkanBufferType ToVulkanBufferType(BufferType type) const;
        bool IsHostVisible(BufferUsage usage) const;

        std::shared_ptr<spdlog::logger> logger;
        VulkanBufferModule* bufferModule;
    };
}
