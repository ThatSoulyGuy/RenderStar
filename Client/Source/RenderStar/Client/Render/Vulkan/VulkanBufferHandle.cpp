#include "RenderStar/Client/Render/Vulkan/VulkanBufferHandle.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    VulkanBufferHandle::VulkanBufferHandle(
        VulkanBufferModule* module,
        VulkanBuffer buf,
        BufferType bufferType,
        BufferUsage bufferUsage)
        : bufferModule(module)
        , buffer(buf)
        , type(bufferType)
        , usage(bufferUsage)
    {
    }

    VulkanBufferHandle::~VulkanBufferHandle()
    {
        if (bufferModule && buffer.buffer != VK_NULL_HANDLE)
            bufferModule->DestroyBuffer(buffer);
    }

    void VulkanBufferHandle::SetData(const void* data, size_t size)
    {
        if (usage == BufferUsage::DYNAMIC)
            bufferModule->UploadBufferData(buffer, data, size, 0);
        else
            bufferModule->UploadBufferDataStaged(buffer, data, size, 0);
    }

    void VulkanBufferHandle::SetSubData(const void* data, size_t size, size_t offset)
    {
        if (usage == BufferUsage::DYNAMIC)
            bufferModule->UploadBufferData(buffer, data, size, offset);
        else
            bufferModule->UploadBufferDataStaged(buffer, data, size, offset);
    }

    size_t VulkanBufferHandle::GetSize() const
    {
        return buffer.size;
    }

    BufferType VulkanBufferHandle::GetType() const
    {
        return type;
    }

    BufferUsage VulkanBufferHandle::GetUsage() const
    {
        return usage;
    }

    bool VulkanBufferHandle::IsValid() const
    {
        return buffer.buffer != VK_NULL_HANDLE;
    }

    VkBuffer VulkanBufferHandle::GetVulkanBuffer() const
    {
        return buffer.buffer;
    }

    const VulkanBuffer& VulkanBufferHandle::GetBuffer() const
    {
        return buffer;
    }
}
