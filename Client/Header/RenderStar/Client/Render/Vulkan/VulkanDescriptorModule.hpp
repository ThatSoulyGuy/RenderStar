#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <vector>
#include <unordered_map>
#include <string>

namespace RenderStar::Client::Render::Vulkan
{
    struct VulkanDescriptorSetLayout
    {
        VkDescriptorSetLayout layout;
        std::vector<VkDescriptorSetLayoutBinding> bindings;
    };

    struct VulkanDescriptorSet
    {
        VkDescriptorSet set;
        VkDescriptorSetLayout layout;
    };

    class VulkanDescriptorModule
    {
    public:

        VulkanDescriptorModule();
        ~VulkanDescriptorModule();

        void Create(VkDevice device, uint32_t maxFramesInFlight);
        void Destroy();

        VulkanDescriptorSetLayout CreateDescriptorSetLayout(
            const std::vector<VkDescriptorSetLayoutBinding>& bindings);

        void DestroyDescriptorSetLayout(VulkanDescriptorSetLayout& layout);

        VkDescriptorPool CreateDescriptorPool(
            const std::vector<VkDescriptorPoolSize>& poolSizes,
            uint32_t maxSets);

        void DestroyDescriptorPool(VkDescriptorPool& pool);

        std::vector<VkDescriptorSet> AllocateDescriptorSets(
            VkDescriptorPool pool,
            VkDescriptorSetLayout layout,
            uint32_t count);

        void UpdateDescriptorSetBuffer(
            VkDescriptorSet set,
            uint32_t binding,
            VkBuffer buffer,
            VkDeviceSize offset,
            VkDeviceSize range,
            VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

        void UpdateDescriptorSetImage(
            VkDescriptorSet set,
            uint32_t binding,
            VkImageView imageView,
            VkSampler sampler,
            VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        VkDescriptorSetLayout GetOrCreateMVPLayout();

    private:

        std::shared_ptr<spdlog::logger> logger;
        VkDevice device;
        uint32_t maxFramesInFlight;
        VulkanDescriptorSetLayout mvpLayout;
        bool mvpLayoutCreated;
    };
}
