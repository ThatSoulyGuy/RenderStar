#include "RenderStar/Client/Render/Vulkan/VulkanCommandModule.hpp"
#include <array>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanCommandModule::VulkanCommandModule()
        : logger(spdlog::default_logger())
        , commandPool(VK_NULL_HANDLE)
        , currentCommandBuffer(VK_NULL_HANDLE)
    {
    }

    VulkanCommandModule::~VulkanCommandModule()
    {
    }

    void VulkanCommandModule::CreateCommandPool(VkDevice device, uint32_t graphicsQueueFamily)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = graphicsQueueFamily;

        VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);

        if (result != VK_SUCCESS)
        {
            logger->error("Failed to create command pool: {}", static_cast<int>(result));
            return;
        }

        logger->info("Command pool created");
    }

    void VulkanCommandModule::AllocateCommandBuffers(VkDevice device, uint32_t count)
    {
        commandBuffers.resize(count);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = count;

        VkResult result = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());

        if (result != VK_SUCCESS)
        {
            logger->error("Failed to allocate command buffers: {}", static_cast<int>(result));
            return;
        }

        logger->info("Allocated {} command buffers", count);
    }

    void VulkanCommandModule::BeginRecording(VkCommandBuffer commandBuffer)
    {
        currentCommandBuffer = commandBuffer;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);

        if (result != VK_SUCCESS)
            logger->error("Failed to begin recording command buffer: {}", static_cast<int>(result));
    }

    void VulkanCommandModule::BeginRenderPass(
        VkRenderPass renderPass,
        VkFramebuffer framebuffer,
        int32_t width,
        int32_t height
    )
    {
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { { 0.39f, 0.58f, 0.93f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(currentCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = static_cast<float>(height);
        viewport.width = static_cast<float>(width);
        viewport.height = -static_cast<float>(height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(currentCommandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        vkCmdSetScissor(currentCommandBuffer, 0, 1, &scissor);
    }

    void VulkanCommandModule::EndRenderPass()
    {
        vkCmdEndRenderPass(currentCommandBuffer);
    }

    void VulkanCommandModule::EndRecording()
    {
        VkResult result = vkEndCommandBuffer(currentCommandBuffer);

        if (result != VK_SUCCESS)
            logger->error("Failed to end recording command buffer: {}", static_cast<int>(result));

        currentCommandBuffer = VK_NULL_HANDLE;
    }

    void VulkanCommandModule::Destroy(VkDevice device)
    {
        commandBuffers.clear();

        if (commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device, commandPool, nullptr);
            commandPool = VK_NULL_HANDLE;
            logger->info("Command pool destroyed");
        }
    }

    VkCommandPool VulkanCommandModule::GetCommandPool() const
    {
        return commandPool;
    }

    const std::vector<VkCommandBuffer>& VulkanCommandModule::GetCommandBuffers() const
    {
        return commandBuffers;
    }

    VkCommandBuffer VulkanCommandModule::GetCommandBuffer(uint32_t index) const
    {
        return commandBuffers[index];
    }

    VkCommandBuffer VulkanCommandModule::GetCurrentCommandBuffer() const
    {
        return currentCommandBuffer;
    }
}
