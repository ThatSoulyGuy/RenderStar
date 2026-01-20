#pragma once

#include "RenderStar/Client/Render/Command/IRenderCommandBuffer.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanShaderProgram;
    class VulkanMesh;
    class VulkanUniformBinding;

    class VulkanCommandBufferAdapter : public IRenderCommandBuffer
    {
    public:

        VulkanCommandBufferAdapter();
        ~VulkanCommandBufferAdapter() override;

        void SetCommandBuffer(VkCommandBuffer commandBuffer);
        void SetFrameIndex(int32_t index);

        void Begin() override;
        void End() override;

        void BindPipeline(IShaderProgram* shader) override;
        void BindVertexBuffer(IBufferHandle* buffer, uint32_t binding) override;
        void BindIndexBuffer(IBufferHandle* buffer, IndexType type) override;
        void BindUniformSet(IUniformBindingHandle* uniforms, uint32_t set) override;

        void Draw(uint32_t vertexCount, uint32_t firstVertex) override;
        void DrawIndexed(uint32_t indexCount, uint32_t firstIndex) override;
        void DrawMesh(IMesh* mesh) override;

        void SetViewport(float x, float y, float width, float height) override;
        void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) override;

        void Reset() override;
        bool IsRecording() const override;

        VkCommandBuffer GetNativeHandle() const;

    private:

        std::shared_ptr<spdlog::logger> logger;
        VkCommandBuffer commandBuffer;
        VulkanShaderProgram* currentShader;
        bool recording;
        int32_t frameIndex;
        IndexType currentIndexType;
    };
}
