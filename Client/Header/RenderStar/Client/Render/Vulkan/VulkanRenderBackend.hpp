#pragma once

#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanInstanceModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanSurfaceModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanDeviceModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanSwapchainModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanRenderPassModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanFramebufferModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanCommandModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanSyncModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanMemoryModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanBufferModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanShaderModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanDescriptorModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanBufferManager.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanShaderManager.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanUniformManager.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanCommandQueue.hpp"
#include <spdlog/spdlog.h>
#include <vector>
#include <memory>

struct GLFWwindow;

namespace RenderStar::Client::Render::Vulkan
{
    struct VulkanDrawCommand
    {
        IShaderProgram* shader;
        IUniformBindingHandle* uniformBinding;
        int32_t frameIndex;
        IMesh* mesh;
    };

    class VulkanRenderBackend : public IRenderBackend
    {
    public:

        static constexpr int32_t MAX_FRAMES_IN_FLIGHT = 2;
        static constexpr VkFormat DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;

        VulkanRenderBackend();

        ~VulkanRenderBackend() override;

        RenderBackend GetType() const override;
        const BackendCapabilities& GetCapabilities() const override;

        void Initialize(GLFWwindow* windowHandle, int32_t width, int32_t height) override;

        void Destroy() override;

        void BeginFrame() override;

        void EndFrame() override;

        void WaitIdle() override;

        void OnResize(int32_t width, int32_t height) override;

        int32_t GetWidth() const override;

        int32_t GetHeight() const override;

        int32_t GetCurrentFrame() const override;

        int32_t GetMaxFramesInFlight() const override;

        IBufferManager* GetBufferManager() override;
        IShaderManager* GetShaderManager() override;
        IUniformManager* GetUniformManager() override;
        IRenderCommandQueue* GetCommandQueue() override;

        void SubmitDrawCommand(IShaderProgram* shader, IUniformBindingHandle* uniformBinding, int32_t frameIndex, IMesh* mesh) override;
        void ExecuteDrawCommands() override;

        bool IsInitialized() const override;

    private:

        void RecreateSwapchain();
        void CreateDepthResources();
        void DestroyDepthResources();

        std::shared_ptr<spdlog::logger> logger;
        GLFWwindow* window;
        int32_t width;
        int32_t height;
        int32_t currentFrame;
        uint32_t currentImageIndex;
        bool initialized;
        bool framebufferResized;
        BackendCapabilities capabilities;
        std::vector<VulkanDrawCommand> drawCommands;

        VulkanInstanceModule instanceModule;
        VulkanSurfaceModule surfaceModule;
        VulkanDeviceModule deviceModule;
        VulkanSwapchainModule swapchainModule;
        VulkanRenderPassModule renderPassModule;
        VulkanFramebufferModule framebufferModule;
        VulkanCommandModule commandModule;
        VulkanSyncModule syncModule;
        VulkanMemoryModule memoryModule;
        VulkanBufferModule bufferModule;
        VulkanShaderModule shaderModule;
        VulkanDescriptorModule descriptorModule;

        std::unique_ptr<VulkanBufferManager> bufferManager;
        std::unique_ptr<VulkanShaderManager> shaderManager;
        std::unique_ptr<VulkanUniformManager> uniformManager;
        std::unique_ptr<VulkanCommandQueue> commandQueue;

        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
    };
}
