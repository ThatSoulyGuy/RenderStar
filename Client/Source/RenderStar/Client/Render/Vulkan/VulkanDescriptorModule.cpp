#include "RenderStar/Client/Render/Vulkan/VulkanDescriptorModule.hpp"
#include <stdexcept>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanDescriptorModule::VulkanDescriptorModule()
        : logger(spdlog::default_logger()->clone("VulkanDescriptorModule"))
        , device(VK_NULL_HANDLE)
        , maxFramesInFlight(2)
        , mvpLayout{}
        , mvpLayoutCreated(false)
    {
    }

    VulkanDescriptorModule::~VulkanDescriptorModule()
    {
        Destroy();
    }

    void VulkanDescriptorModule::Create(VkDevice deviceHandle, uint32_t framesInFlight)
    {
        device = deviceHandle;
        maxFramesInFlight = framesInFlight;
        logger->info("Vulkan descriptor module created");
    }

    void VulkanDescriptorModule::Destroy()
    {
        if (device == VK_NULL_HANDLE)
            return;

        if (mvpLayoutCreated)
        {
            DestroyDescriptorSetLayout(mvpLayout);
            mvpLayoutCreated = false;
        }

        device = VK_NULL_HANDLE;
        logger->info("Vulkan descriptor module destroyed");
    }

    VulkanDescriptorSetLayout VulkanDescriptorModule::CreateDescriptorSetLayout(
        const std::vector<VkDescriptorSetLayoutBinding>& bindings)
    {
        VulkanDescriptorSetLayout result{};
        result.bindings = bindings;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkResult vkResult = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &result.layout);

        if (vkResult != VK_SUCCESS)
        {
            logger->error("Failed to create descriptor set layout");
            throw std::runtime_error("Failed to create descriptor set layout");
        }

        return result;
    }

    void VulkanDescriptorModule::DestroyDescriptorSetLayout(VulkanDescriptorSetLayout& layout)
    {
        if (layout.layout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(device, layout.layout, nullptr);
            layout.layout = VK_NULL_HANDLE;
            layout.bindings.clear();
        }
    }

    VkDescriptorPool VulkanDescriptorModule::CreateDescriptorPool(
        const std::vector<VkDescriptorPoolSize>& poolSizes,
        uint32_t maxSets)
    {
        VkDescriptorPool pool = VK_NULL_HANDLE;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;

        VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);

        if (result != VK_SUCCESS)
        {
            logger->error("Failed to create descriptor pool");
            throw std::runtime_error("Failed to create descriptor pool");
        }

        return pool;
    }

    void VulkanDescriptorModule::DestroyDescriptorPool(VkDescriptorPool& pool)
    {
        if (pool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device, pool, nullptr);
            pool = VK_NULL_HANDLE;
        }
    }

    std::vector<VkDescriptorSet> VulkanDescriptorModule::AllocateDescriptorSets(
        VkDescriptorPool pool,
        VkDescriptorSetLayout layout,
        uint32_t count)
    {
        std::vector<VkDescriptorSetLayout> layouts(count, layout);
        std::vector<VkDescriptorSet> sets(count);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = count;
        allocInfo.pSetLayouts = layouts.data();

        VkResult result = vkAllocateDescriptorSets(device, &allocInfo, sets.data());

        if (result != VK_SUCCESS)
        {
            logger->error("Failed to allocate descriptor sets");
            throw std::runtime_error("Failed to allocate descriptor sets");
        }

        return sets;
    }

    void VulkanDescriptorModule::UpdateDescriptorSetBuffer(
        VkDescriptorSet set,
        uint32_t binding,
        VkBuffer buffer,
        VkDeviceSize offset,
        VkDeviceSize range,
        VkDescriptorType type)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = offset;
        bufferInfo.range = range;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = set;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = type;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    void VulkanDescriptorModule::UpdateDescriptorSetImage(
        VkDescriptorSet set,
        uint32_t binding,
        VkImageView imageView,
        VkSampler sampler,
        VkImageLayout layout)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = layout;
        imageInfo.imageView = imageView;
        imageInfo.sampler = sampler;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = set;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    VkDescriptorSetLayout VulkanDescriptorModule::GetOrCreateMVPLayout()
    {
        if (mvpLayoutCreated)
            return mvpLayout.layout;

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        mvpLayout = CreateDescriptorSetLayout({ uboLayoutBinding });
        mvpLayoutCreated = true;

        logger->info("Created MVP descriptor set layout");

        return mvpLayout.layout;
    }
}
