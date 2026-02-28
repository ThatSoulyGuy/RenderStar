#include "RenderStar/Client/Render/Vulkan/VulkanUniformBinding.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanDescriptorModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanBufferHandle.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanTextureHandle.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    VulkanUniformBinding::VulkanUniformBinding(
        const std::vector<VkDescriptorSet>& descriptorSets,
        VulkanDescriptorModule* descriptorModule)
        : descriptorSets(descriptorSets)
        , descriptorModule(descriptorModule)
        , destroyed(false)
    {
    }

    VulkanUniformBinding::~VulkanUniformBinding()
    {
        Destroy();
    }

    void VulkanUniformBinding::Bind(int32_t frameIndex)
    {
    }

    void VulkanUniformBinding::UpdateBuffer(int32_t binding, IBufferHandle* buffer, size_t size, int32_t frameIndex)
    {
        auto* vulkanBuffer = static_cast<VulkanBufferHandle*>(buffer);
        if (vulkanBuffer == nullptr)
            return;

        if (frameIndex >= 0 && frameIndex < static_cast<int32_t>(descriptorSets.size()))
        {
            descriptorModule->UpdateDescriptorSetBuffer(
                descriptorSets[frameIndex],
                binding,
                vulkanBuffer->GetVulkanBuffer(),
                0,
                size);
        }
        else
        {
            for (VkDescriptorSet descriptorSet : descriptorSets)
            {
                descriptorModule->UpdateDescriptorSetBuffer(
                    descriptorSet,
                    binding,
                    vulkanBuffer->GetVulkanBuffer(),
                    0,
                    size);
            }
        }
    }

    void VulkanUniformBinding::UpdateTexture(int32_t binding, ITextureHandle* texture, int32_t frameIndex)
    {
        auto* vulkanTexture = static_cast<VulkanTextureHandle*>(texture);
        if (vulkanTexture == nullptr)
            return;

        if (frameIndex >= 0 && frameIndex < static_cast<int32_t>(descriptorSets.size()))
        {
            descriptorModule->UpdateDescriptorSetImage(
                descriptorSets[frameIndex],
                binding,
                vulkanTexture->GetImageView(),
                vulkanTexture->GetSampler());
        }
        else
        {
            for (VkDescriptorSet descriptorSet : descriptorSets)
            {
                descriptorModule->UpdateDescriptorSetImage(
                    descriptorSet,
                    binding,
                    vulkanTexture->GetImageView(),
                    vulkanTexture->GetSampler());
            }
        }
    }

    uint64_t VulkanUniformBinding::GetNativeHandle(int32_t frameIndex) const
    {
        if (frameIndex < 0 || frameIndex >= static_cast<int32_t>(descriptorSets.size()))
            return 0;

        return reinterpret_cast<uint64_t>(descriptorSets[frameIndex]);
    }

    void VulkanUniformBinding::Destroy()
    {
        if (!destroyed)
            destroyed = true;
    }

    bool VulkanUniformBinding::IsDestroyed() const
    {
        return destroyed;
    }

    const std::vector<VkDescriptorSet>& VulkanUniformBinding::GetDescriptorSets() const
    {
        return descriptorSets;
    }

    int32_t VulkanUniformBinding::GetFrameCount() const
    {
        return static_cast<int32_t>(descriptorSets.size());
    }
}
