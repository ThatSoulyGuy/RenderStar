#include "RenderStar/Client/Render/Vulkan/VulkanMesh.hpp"
#include "RenderStar/Client/Render/Resource/IGraphicsResourceManager.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    VulkanMesh::VulkanMesh()
        : bufferModule(nullptr)
        , vertexBuffer{}
        , indexBuffer{}
        , vertexLayout{}
        , primitiveType(PrimitiveType::TRIANGLES)
        , indexType(IndexType::UINT32)
        , vertexCount(0)
        , indexCount(0)
        , hasIndices(false)
    {
    }

    VulkanMesh::~VulkanMesh()
    {
        if (!released)
            Release();
    }

    void VulkanMesh::Initialize(
        VulkanBufferModule* module,
        IGraphicsResourceManager& manager,
        const VertexLayout& layout,
        PrimitiveType primitive)
    {
        bufferModule = module;
        vertexLayout = layout;
        primitiveType = primitive;
        manager.Track(this);
    }

    void VulkanMesh::Release()
    {
        if (released)
            return;

        if (bufferModule)
        {
            if (vertexBuffer.buffer != VK_NULL_HANDLE)
            {
                bufferModule->DestroyBuffer(vertexBuffer);
                vertexBuffer.buffer = VK_NULL_HANDLE;
            }

            if (indexBuffer.buffer != VK_NULL_HANDLE)
            {
                bufferModule->DestroyBuffer(indexBuffer);
                indexBuffer.buffer = VK_NULL_HANDLE;
            }
        }

        released = true;
    }

    GraphicsResourceType VulkanMesh::GetResourceType() const
    {
        return GraphicsResourceType::MESH;
    }

    void VulkanMesh::SetVertexData(const void* data, size_t size)
    {
        if (released)
            return;

        if (vertexBuffer.buffer != VK_NULL_HANDLE)
            bufferModule->DestroyBuffer(vertexBuffer);

        vertexBuffer = bufferModule->CreateBuffer(VulkanBufferType::VERTEX, size, false);
        bufferModule->UploadBufferDataStaged(vertexBuffer, data, size, 0);

        vertexCount = static_cast<int32_t>(size / vertexLayout.stride);
    }

    void VulkanMesh::SetIndexData(const void* data, size_t size, IndexType type)
    {
        if (released)
            return;

        if (indexBuffer.buffer != VK_NULL_HANDLE)
            bufferModule->DestroyBuffer(indexBuffer);

        indexType = type;
        indexBuffer = bufferModule->CreateBuffer(VulkanBufferType::INDEX, size, false);
        bufferModule->UploadBufferDataStaged(indexBuffer, data, size, 0);

        size_t indexSize = (type == IndexType::UINT16) ? sizeof(uint16_t) : sizeof(uint32_t);
        indexCount = static_cast<int32_t>(size / indexSize);
        hasIndices = true;
    }

    int32_t VulkanMesh::GetVertexCount() const
    {
        return vertexCount;
    }

    int32_t VulkanMesh::GetIndexCount() const
    {
        return indexCount;
    }

    const VertexLayout& VulkanMesh::GetVertexLayout() const
    {
        return vertexLayout;
    }

    PrimitiveType VulkanMesh::GetPrimitiveType() const
    {
        return primitiveType;
    }

    bool VulkanMesh::HasIndices() const
    {
        return hasIndices;
    }

    bool VulkanMesh::IsValid() const
    {
        return !released && vertexBuffer.buffer != VK_NULL_HANDLE && vertexCount > 0;
    }

    VkBuffer VulkanMesh::GetVertexBuffer() const
    {
        return vertexBuffer.buffer;
    }

    VkBuffer VulkanMesh::GetIndexBuffer() const
    {
        return indexBuffer.buffer;
    }

    VkIndexType VulkanMesh::GetVulkanIndexType() const
    {
        return (indexType == IndexType::UINT16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
    }

    void VulkanMesh::RecordDrawCommands(VkCommandBuffer commandBuffer)
    {
        if (released)
            return;

        VkBuffer vertexBuffers[] = { vertexBuffer.buffer };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        if (hasIndices)
        {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, GetVulkanIndexType());
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indexCount), 1, 0, 0, 0);
        }
        else
        {
            vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertexCount), 1, 0, 0);
        }
    }
}
