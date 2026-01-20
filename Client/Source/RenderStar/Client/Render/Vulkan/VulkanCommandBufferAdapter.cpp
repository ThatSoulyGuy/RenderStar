#include "RenderStar/Client/Render/Vulkan/VulkanCommandBufferAdapter.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanShaderProgram.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanMesh.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanUniformBinding.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanBufferHandle.hpp"
#include "RenderStar/Client/Render/Resource/IMesh.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    VulkanCommandBufferAdapter::VulkanCommandBufferAdapter()
        : logger(spdlog::default_logger())
        , commandBuffer(VK_NULL_HANDLE)
        , currentShader(nullptr)
        , recording(false)
        , frameIndex(0)
        , currentIndexType(IndexType::UINT32)
    {
    }

    VulkanCommandBufferAdapter::~VulkanCommandBufferAdapter()
    {
    }

    void VulkanCommandBufferAdapter::SetCommandBuffer(VkCommandBuffer newCommandBuffer)
    {
        commandBuffer = newCommandBuffer;
    }

    void VulkanCommandBufferAdapter::SetFrameIndex(int32_t index)
    {
        frameIndex = index;
    }

    void VulkanCommandBufferAdapter::Begin()
    {
        if (commandBuffer == VK_NULL_HANDLE)
            return;

        recording = true;
        currentShader = nullptr;
    }

    void VulkanCommandBufferAdapter::End()
    {
        recording = false;
    }

    void VulkanCommandBufferAdapter::BindPipeline(IShaderProgram* shader)
    {
        if (!recording || commandBuffer == VK_NULL_HANDLE)
            return;

        auto* vkShader = static_cast<VulkanShaderProgram*>(shader);
        if (vkShader != nullptr)
        {
            vkShader->BindPipeline(commandBuffer);
            currentShader = vkShader;
        }
    }

    void VulkanCommandBufferAdapter::BindVertexBuffer(IBufferHandle* buffer, uint32_t binding)
    {
        if (!recording || commandBuffer == VK_NULL_HANDLE)
            return;

        auto* vkBuffer = static_cast<VulkanBufferHandle*>(buffer);
        if (vkBuffer != nullptr)
        {
            VkBuffer buffers[] = { vkBuffer->GetVulkanBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, binding, 1, buffers, offsets);
        }
    }

    void VulkanCommandBufferAdapter::BindIndexBuffer(IBufferHandle* buffer, IndexType type)
    {
        if (!recording || commandBuffer == VK_NULL_HANDLE)
            return;

        auto* vkBuffer = static_cast<VulkanBufferHandle*>(buffer);
        if (vkBuffer != nullptr)
        {
            VkIndexType vkIndexType = (type == IndexType::UINT16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
            vkCmdBindIndexBuffer(commandBuffer, vkBuffer->GetVulkanBuffer(), 0, vkIndexType);
            currentIndexType = type;
        }
    }

    void VulkanCommandBufferAdapter::BindUniformSet(IUniformBindingHandle* uniforms, uint32_t set)
    {
        if (!recording || commandBuffer == VK_NULL_HANDLE || currentShader == nullptr)
            return;

        auto* vkBinding = static_cast<VulkanUniformBinding*>(uniforms);
        if (vkBinding != nullptr)
        {
            VkDescriptorSet descriptorSet = reinterpret_cast<VkDescriptorSet>(vkBinding->GetNativeHandle(frameIndex));
            currentShader->BindDescriptorSet(commandBuffer, descriptorSet);
        }
    }

    void VulkanCommandBufferAdapter::Draw(uint32_t vertexCount, uint32_t firstVertex)
    {
        if (!recording || commandBuffer == VK_NULL_HANDLE)
            return;

        vkCmdDraw(commandBuffer, vertexCount, 1, firstVertex, 0);
    }

    void VulkanCommandBufferAdapter::DrawIndexed(uint32_t indexCount, uint32_t firstIndex)
    {
        if (!recording || commandBuffer == VK_NULL_HANDLE)
            return;

        vkCmdDrawIndexed(commandBuffer, indexCount, 1, firstIndex, 0, 0);
    }

    void VulkanCommandBufferAdapter::DrawMesh(IMesh* mesh)
    {
        if (!recording || commandBuffer == VK_NULL_HANDLE || mesh == nullptr)
            return;

        auto* vkMesh = static_cast<VulkanMesh*>(mesh);
        if (vkMesh != nullptr && vkMesh->IsValid())
            vkMesh->RecordDrawCommands(commandBuffer);
    }

    void VulkanCommandBufferAdapter::SetViewport(float x, float y, float width, float height)
    {
        if (!recording || commandBuffer == VK_NULL_HANDLE)
            return;

        VkViewport viewport{};
        viewport.x = x;
        viewport.y = y + height;
        viewport.width = width;
        viewport.height = -height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    }

    void VulkanCommandBufferAdapter::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        if (!recording || commandBuffer == VK_NULL_HANDLE)
            return;

        VkRect2D scissor{};
        scissor.offset = { x, y };
        scissor.extent = { width, height };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void VulkanCommandBufferAdapter::Reset()
    {
        recording = false;
        currentShader = nullptr;
    }

    bool VulkanCommandBufferAdapter::IsRecording() const
    {
        return recording;
    }

    VkCommandBuffer VulkanCommandBufferAdapter::GetNativeHandle() const
    {
        return commandBuffer;
    }
}
