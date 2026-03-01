#include "RenderStar/Client/Render/Vulkan/VulkanUniformBinding.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanDescriptorModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanBufferHandle.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanTextureHandle.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanRenderTarget.hpp"
#include "RenderStar/Client/Render/Resource/IGraphicsResourceManager.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    VulkanUniformBinding::VulkanUniformBinding(
        const std::vector<VkDescriptorSet>& descriptorSets,
        VulkanDescriptorModule* descriptorModule,
        IGraphicsResourceManager& manager)
        : descriptorSets(descriptorSets)
        , descriptorModule(descriptorModule)
        , destroyed(false)
    {
        manager.Track(this);
    }

    VulkanUniformBinding::~VulkanUniformBinding()
    {
        if (!released)
            Release();
    }

    void VulkanUniformBinding::Release()
    {
        if (released)
            return;

        destroyed = true;
        descriptorSets.clear();
        released = true;
    }

    GraphicsResourceType VulkanUniformBinding::GetResourceType() const
    {
        return GraphicsResourceType::UNIFORM_BINDING;
    }

    void VulkanUniformBinding::Bind(int32_t frameIndex)
    {
    }

    void VulkanUniformBinding::UpdateBuffer(int32_t binding, IBufferHandle* buffer, size_t size, int32_t frameIndex)
    {
        if (released)
            return;

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
        if (released || !texture)
            return;

        VkImageView imageView = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;

        if (auto* vulkanTexture = dynamic_cast<VulkanTextureHandle*>(texture))
        {
            imageView = vulkanTexture->GetImageView();
            sampler = vulkanTexture->GetSampler();
        }
        else if (auto* rtAttachment = dynamic_cast<VulkanRenderTargetAttachment*>(texture))
        {
            imageView = rtAttachment->GetImageView();
            sampler = rtAttachment->GetSampler();
        }

        if (imageView == VK_NULL_HANDLE || sampler == VK_NULL_HANDLE)
            return;

        if (frameIndex >= 0 && frameIndex < static_cast<int32_t>(descriptorSets.size()))
        {
            descriptorModule->UpdateDescriptorSetImage(
                descriptorSets[frameIndex],
                binding,
                imageView,
                sampler);
        }
        else
        {
            for (VkDescriptorSet descriptorSet : descriptorSets)
            {
                descriptorModule->UpdateDescriptorSetImage(
                    descriptorSet,
                    binding,
                    imageView,
                    sampler);
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
