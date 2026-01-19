#include "RenderStar/Client/Render/Vulkan/VulkanRenderBackend.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanShaderProgram.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanMesh.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanUniformBinding.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/IMesh.hpp"
#include "RenderStar/Client/Render/Resource/Vertex.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanRenderBackend::VulkanRenderBackend()
        : logger(spdlog::default_logger())
        , window(nullptr)
        , width(0)
        , height(0)
        , currentFrame(0)
        , currentImageIndex(0)
        , initialized(false)
        , framebufferResized(false)
        , capabilities(BackendCapabilities::ForVulkan())
        , depthImage(VK_NULL_HANDLE)
        , depthImageMemory(VK_NULL_HANDLE)
        , depthImageView(VK_NULL_HANDLE)
    {
    }

    VulkanRenderBackend::~VulkanRenderBackend()
    {
        if (initialized)
            Destroy();
    }

    RenderBackend VulkanRenderBackend::GetType() const
    {
        return RenderBackend::VULKAN;
    }

    const BackendCapabilities& VulkanRenderBackend::GetCapabilities() const
    {
        return capabilities;
    }

    void VulkanRenderBackend::Initialize(GLFWwindow* windowHandle, int32_t initialWidth, int32_t initialHeight)
    {
        window = windowHandle;
        width = initialWidth;
        height = initialHeight;

#ifdef RENDERSTAR_ENABLE_VALIDATION
        bool enableValidation = true;
#else
        bool enableValidation = false;
#endif

        instanceModule.Create(enableValidation);
        surfaceModule.Create(instanceModule.GetInstance(), window);
        deviceModule.Create(instanceModule.GetInstance(), surfaceModule.GetSurface(), enableValidation);

        memoryModule.Create(
            instanceModule.GetInstance(),
            deviceModule.GetPhysicalDevice(),
            deviceModule.GetDevice()
        );

        swapchainModule.Create(
            deviceModule.GetPhysicalDevice(),
            deviceModule.GetDevice(),
            surfaceModule.GetSurface(),
            width,
            height,
            deviceModule.GetGraphicsQueueFamily(),
            deviceModule.GetPresentQueueFamily()
        );

        CreateDepthResources();

        renderPassModule.Create(deviceModule.GetDevice(), swapchainModule.GetImageFormat(), DEPTH_FORMAT);

        framebufferModule.Create(
            deviceModule.GetDevice(),
            swapchainModule.GetImageViews(),
            depthImageView,
            renderPassModule.GetRenderPass(),
            swapchainModule.GetWidth(),
            swapchainModule.GetHeight()
        );

        commandModule.CreateCommandPool(deviceModule.GetDevice(), deviceModule.GetGraphicsQueueFamily());
        commandModule.AllocateCommandBuffers(deviceModule.GetDevice(), MAX_FRAMES_IN_FLIGHT);

        syncModule.Create(deviceModule.GetDevice());

        bufferModule.Create(
            deviceModule.GetDevice(),
            memoryModule.GetAllocator(),
            deviceModule.GetGraphicsQueue(),
            deviceModule.GetGraphicsQueueFamily()
        );

        shaderModule.Create(deviceModule.GetDevice());

        descriptorModule.Create(deviceModule.GetDevice(), MAX_FRAMES_IN_FLIGHT);

        bufferManager = std::make_unique<VulkanBufferManager>();
        bufferManager->Initialize(&bufferModule);

        shaderManager = std::make_unique<VulkanShaderManager>();
        shaderManager->Initialize(
            deviceModule.GetDevice(),
            renderPassModule.GetRenderPass(),
            &shaderModule,
            &descriptorModule,
            Vertex::LAYOUT);

        uniformManager = std::make_unique<VulkanUniformManager>();
        uniformManager->Initialize(&bufferModule, &descriptorModule, MAX_FRAMES_IN_FLIGHT);

        initialized = true;
        logger->info("Vulkan render backend initialized ({}x{})", width, height);
    }

    void VulkanRenderBackend::Destroy()
    {
        vkDeviceWaitIdle(deviceModule.GetDevice());

        uniformManager->Destroy();
        uniformManager.reset();
        shaderManager.reset();
        bufferManager.reset();

        descriptorModule.Destroy();
        shaderModule.Destroy();
        bufferModule.Destroy();

        syncModule.Destroy(deviceModule.GetDevice());
        commandModule.Destroy(deviceModule.GetDevice());
        framebufferModule.Destroy(deviceModule.GetDevice());
        renderPassModule.Destroy(deviceModule.GetDevice());
        DestroyDepthResources();
        swapchainModule.Destroy(deviceModule.GetDevice());
        memoryModule.Destroy();
        deviceModule.Destroy();
        surfaceModule.Destroy(instanceModule.GetInstance());
        instanceModule.Destroy();

        initialized = false;
        logger->info("Vulkan render backend destroyed");
    }

    void VulkanRenderBackend::BeginFrame()
    {
        VkDevice device = deviceModule.GetDevice();
        VkFence inFlightFence = syncModule.GetInFlightFence(currentFrame);

        vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

        VkResult result = vkAcquireNextImageKHR(
            device,
            swapchainModule.GetSwapchain(),
            UINT64_MAX,
            syncModule.GetImageAvailableSemaphore(currentFrame),
            VK_NULL_HANDLE,
            &currentImageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapchain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            logger->error("Failed to acquire swapchain image: {}", static_cast<int>(result));
            return;
        }

        vkResetFences(device, 1, &inFlightFence);

        VkCommandBuffer commandBuffer = commandModule.GetCommandBuffer(currentFrame);
        vkResetCommandBuffer(commandBuffer, 0);

        commandModule.BeginRecording(commandBuffer);
        commandModule.BeginRenderPass(
            renderPassModule.GetRenderPass(),
            framebufferModule.GetFramebuffer(currentImageIndex),
            swapchainModule.GetWidth(),
            swapchainModule.GetHeight()
        );
    }

    void VulkanRenderBackend::EndFrame()
    {
        commandModule.EndRenderPass();
        commandModule.EndRecording();

        VkCommandBuffer commandBuffer = commandModule.GetCommandBuffer(currentFrame);
        VkSemaphore waitSemaphores[] = { syncModule.GetImageAvailableSemaphore(currentFrame) };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSemaphores[] = { syncModule.GetRenderFinishedSemaphore(currentFrame) };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkResult result = vkQueueSubmit(
            deviceModule.GetGraphicsQueue(),
            1,
            &submitInfo,
            syncModule.GetInFlightFence(currentFrame)
        );

        if (result != VK_SUCCESS)
            logger->error("Failed to submit draw command buffer: {}", static_cast<int>(result));

        VkSwapchainKHR swapchains[] = { swapchainModule.GetSwapchain() };

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &currentImageIndex;

        result = vkQueuePresentKHR(deviceModule.GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
        {
            framebufferResized = false;
            RecreateSwapchain();
        }
        else if (result != VK_SUCCESS)
        {
            logger->error("Failed to present swapchain image: {}", static_cast<int>(result));
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void VulkanRenderBackend::WaitIdle()
    {
        vkDeviceWaitIdle(deviceModule.GetDevice());
    }

    void VulkanRenderBackend::OnResize(int32_t newWidth, int32_t newHeight)
    {
        width = newWidth;
        height = newHeight;
        framebufferResized = true;
    }

    void VulkanRenderBackend::RecreateSwapchain()
    {
        int windowWidth = 0;
        int windowHeight = 0;
        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

        while (windowWidth == 0 || windowHeight == 0)
        {
            glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(deviceModule.GetDevice());

        framebufferModule.Destroy(deviceModule.GetDevice());
        DestroyDepthResources();
        swapchainModule.Destroy(deviceModule.GetDevice());

        swapchainModule.Create(
            deviceModule.GetPhysicalDevice(),
            deviceModule.GetDevice(),
            surfaceModule.GetSurface(),
            windowWidth,
            windowHeight,
            deviceModule.GetGraphicsQueueFamily(),
            deviceModule.GetPresentQueueFamily()
        );

        CreateDepthResources();

        framebufferModule.Create(
            deviceModule.GetDevice(),
            swapchainModule.GetImageViews(),
            depthImageView,
            renderPassModule.GetRenderPass(),
            swapchainModule.GetWidth(),
            swapchainModule.GetHeight()
        );

        width = swapchainModule.GetWidth();
        height = swapchainModule.GetHeight();

        logger->info("Swapchain recreated ({}x{})", width, height);
    }

    int32_t VulkanRenderBackend::GetWidth() const
    {
        return width;
    }

    int32_t VulkanRenderBackend::GetHeight() const
    {
        return height;
    }

    int32_t VulkanRenderBackend::GetCurrentFrame() const
    {
        return currentFrame;
    }

    int32_t VulkanRenderBackend::GetMaxFramesInFlight() const
    {
        return MAX_FRAMES_IN_FLIGHT;
    }

    bool VulkanRenderBackend::IsInitialized() const
    {
        return initialized;
    }

    IBufferManager* VulkanRenderBackend::GetBufferManager()
    {
        return bufferManager.get();
    }

    IShaderManager* VulkanRenderBackend::GetShaderManager()
    {
        return shaderManager.get();
    }

    IUniformManager* VulkanRenderBackend::GetUniformManager()
    {
        return uniformManager.get();
    }

    void VulkanRenderBackend::SubmitDrawCommand(IShaderProgram* shader, IUniformBindingHandle* uniformBinding, int32_t frameIndex, IMesh* mesh)
    {
        drawCommands.push_back(VulkanDrawCommand{ shader, uniformBinding, frameIndex, mesh });
    }

    void VulkanRenderBackend::ExecuteDrawCommands()
    {
        VkCommandBuffer commandBuffer = commandModule.GetCommandBuffer(currentFrame);

        for (const auto& command : drawCommands)
        {
            auto* vulkanShader = dynamic_cast<VulkanShaderProgram*>(command.shader);
            auto* vulkanMesh = dynamic_cast<VulkanMesh*>(command.mesh);
            auto* vulkanBinding = dynamic_cast<VulkanUniformBinding*>(command.uniformBinding);

            if (vulkanShader)
                vulkanShader->BindPipeline(commandBuffer);

            if (vulkanShader && vulkanBinding && command.frameIndex >= 0 && command.frameIndex < vulkanBinding->GetFrameCount())
            {
                VkDescriptorSet descriptorSet = vulkanBinding->GetDescriptorSets()[command.frameIndex];
                vulkanShader->BindDescriptorSet(commandBuffer, descriptorSet);
            }

            if (vulkanMesh && vulkanMesh->IsValid())
                vulkanMesh->RecordDrawCommands(commandBuffer);
        }

        drawCommands.clear();
    }

    void VulkanRenderBackend::CreateDepthResources()
    {
        VkDevice device = deviceModule.GetDevice();
        int32_t swapWidth = swapchainModule.GetWidth();
        int32_t swapHeight = swapchainModule.GetHeight();

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(swapWidth);
        imageInfo.extent.height = static_cast<uint32_t>(swapHeight);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = DEPTH_FORMAT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateImage(device, &imageInfo, nullptr, &depthImage);
        if (result != VK_SUCCESS)
        {
            logger->error("Failed to create depth image: {}", static_cast<int>(result));
            return;
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, depthImage, &memRequirements);

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(deviceModule.GetPhysicalDevice(), &memProperties);

        uint32_t memoryTypeIndex = UINT32_MAX;
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((memRequirements.memoryTypeBits & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
            {
                memoryTypeIndex = i;
                break;
            }
        }

        if (memoryTypeIndex == UINT32_MAX)
        {
            logger->error("Failed to find suitable memory type for depth buffer");
            return;
        }

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        result = vkAllocateMemory(device, &allocInfo, nullptr, &depthImageMemory);
        if (result != VK_SUCCESS)
        {
            logger->error("Failed to allocate depth image memory: {}", static_cast<int>(result));
            return;
        }

        vkBindImageMemory(device, depthImage, depthImageMemory, 0);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = depthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = DEPTH_FORMAT;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(device, &viewInfo, nullptr, &depthImageView);
        if (result != VK_SUCCESS)
        {
            logger->error("Failed to create depth image view: {}", static_cast<int>(result));
            return;
        }

        logger->info("Depth resources created ({}x{})", swapWidth, swapHeight);
    }

    void VulkanRenderBackend::DestroyDepthResources()
    {
        VkDevice device = deviceModule.GetDevice();

        if (depthImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, depthImageView, nullptr);
            depthImageView = VK_NULL_HANDLE;
        }

        if (depthImage != VK_NULL_HANDLE)
        {
            vkDestroyImage(device, depthImage, nullptr);
            depthImage = VK_NULL_HANDLE;
        }

        if (depthImageMemory != VK_NULL_HANDLE)
        {
            vkFreeMemory(device, depthImageMemory, nullptr);
            depthImageMemory = VK_NULL_HANDLE;
        }
    }
}
