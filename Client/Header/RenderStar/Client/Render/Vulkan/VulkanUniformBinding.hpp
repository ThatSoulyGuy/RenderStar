#pragma once

#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanDescriptorModule;

    class VulkanUniformBinding : public IUniformBindingHandle
    {
    public:

        VulkanUniformBinding(const std::vector<VkDescriptorSet>& descriptorSets, VulkanDescriptorModule* descriptorModule, IGraphicsResourceManager& resourceManager);
        ~VulkanUniformBinding() override;

        void Release() override;
        GraphicsResourceType GetResourceType() const override;

        void Bind(int32_t frameIndex) override;
        void UpdateBuffer(int32_t binding, IBufferHandle* buffer, size_t size, int32_t frameIndex = -1) override;
        void UpdateTexture(int32_t binding, ITextureHandle* texture, int32_t frameIndex = -1) override;
        uint64_t GetNativeHandle(int32_t frameIndex) const override;
        void Destroy() override;
        bool IsDestroyed() const override;

        const std::vector<VkDescriptorSet>& GetDescriptorSets() const;
        int32_t GetFrameCount() const;

    private:

        std::vector<VkDescriptorSet> descriptorSets;
        VulkanDescriptorModule* descriptorModule;
        bool destroyed;
    };
}
