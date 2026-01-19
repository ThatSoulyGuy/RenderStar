#pragma once

#include "RenderStar/Client/Render/Resource/IMesh.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanBufferModule.hpp"
#include <vulkan/vulkan.h>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanMesh : public IMesh
    {
    public:

        VulkanMesh();
        ~VulkanMesh() override;

        void Initialize(
            VulkanBufferModule* bufferModule,
            const VertexLayout& layout,
            PrimitiveType primitive = PrimitiveType::TRIANGLES);

        void Bind() override;
        void Unbind() override;

        void Draw() override;
        void DrawInstanced(int32_t instanceCount) override;

        void SetVertexData(const void* data, size_t size) override;
        void SetIndexData(const void* data, size_t size, IndexType indexType) override;

        int32_t GetVertexCount() const override;
        int32_t GetIndexCount() const override;

        const VertexLayout& GetVertexLayout() const override;
        PrimitiveType GetPrimitiveType() const override;

        bool HasIndices() const override;
        bool IsValid() const override;

        VkBuffer GetVertexBuffer() const;
        VkBuffer GetIndexBuffer() const;
        VkIndexType GetVulkanIndexType() const;

        void RecordDrawCommands(VkCommandBuffer commandBuffer);

    private:

        VulkanBufferModule* bufferModule;
        VulkanBuffer vertexBuffer;
        VulkanBuffer indexBuffer;
        VertexLayout vertexLayout;
        PrimitiveType primitiveType;
        IndexType indexType;
        int32_t vertexCount;
        int32_t indexCount;
        bool hasIndices;
    };
}
