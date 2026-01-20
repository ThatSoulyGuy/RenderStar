#include "RenderStar/Client/Render/Vulkan/VulkanCommandQueue.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanCommandModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanCommandBufferAdapter.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    VulkanCommandQueue::VulkanCommandQueue()
        : logger(spdlog::default_logger())
        , commandModule(nullptr)
        , currentFrameIndex(0)
        , maxFramesInFlight(0)
    {
    }

    VulkanCommandQueue::~VulkanCommandQueue()
    {
        adapters.clear();
    }

    void VulkanCommandQueue::Initialize(VulkanCommandModule* module, int32_t framesInFlight)
    {
        commandModule = module;
        maxFramesInFlight = framesInFlight;

        adapters.clear();
        adapters.reserve(framesInFlight);

        for (int32_t i = 0; i < framesInFlight; ++i)
            adapters.push_back(std::make_unique<VulkanCommandBufferAdapter>());
    }

    IRenderCommandBuffer* VulkanCommandQueue::AcquireCommandBuffer()
    {
        if (commandModule == nullptr || currentFrameIndex >= static_cast<int32_t>(adapters.size()))
            return nullptr;

        VkCommandBuffer vkCommandBuffer = commandModule->GetCommandBuffer(currentFrameIndex);
        VulkanCommandBufferAdapter* adapter = adapters[currentFrameIndex].get();
        adapter->SetCommandBuffer(vkCommandBuffer);
        adapter->SetFrameIndex(currentFrameIndex);
        adapter->Reset();

        return adapter;
    }

    void VulkanCommandQueue::ReleaseCommandBuffer(IRenderCommandBuffer* buffer)
    {
    }

    void VulkanCommandQueue::Submit(IRenderCommandBuffer* buffer)
    {
        ReleaseCommandBuffer(buffer);
    }

    int32_t VulkanCommandQueue::GetCurrentFrameIndex() const
    {
        return currentFrameIndex;
    }

    void VulkanCommandQueue::SetCurrentFrameIndex(int32_t index)
    {
        currentFrameIndex = index;
    }
}
