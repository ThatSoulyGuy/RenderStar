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

        VulkanUniformBinding(const std::vector<VkDescriptorSet>& descriptorSets, VulkanDescriptorModule* descriptorModule);
        ~VulkanUniformBinding() override;

        void Bind(int32_t frameIndex) override;
        void UpdateBuffer(int32_t binding, IBufferHandle* buffer, size_t size) override;
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
