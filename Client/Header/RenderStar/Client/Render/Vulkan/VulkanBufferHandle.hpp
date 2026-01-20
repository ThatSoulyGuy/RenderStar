#pragma once

#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanBufferModule.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanBufferHandle : public IBufferHandle
    {
    public:

        VulkanBufferHandle(
            VulkanBufferModule* bufferModule,
            VulkanBuffer buffer,
            BufferType type,
            BufferUsage usage);

        ~VulkanBufferHandle() override;

        void SetData(const void* data, size_t size) override;
        void SetSubData(const void* data, size_t size, size_t offset) override;

        size_t GetSize() const override;
        BufferType GetType() const override;
        BufferUsage GetUsage() const override;

        bool IsValid() const override;

        VkBuffer GetVulkanBuffer() const;
        const VulkanBuffer& GetBuffer() const;

    private:

        VulkanBufferModule* bufferModule;
        VulkanBuffer buffer;
        BufferType type;
        BufferUsage usage;
    };
}
