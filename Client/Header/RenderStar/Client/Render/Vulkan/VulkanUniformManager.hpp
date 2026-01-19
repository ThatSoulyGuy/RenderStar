#pragma once

#include "RenderStar/Client/Render/Resource/IUniformManager.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanBufferModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanDescriptorModule.hpp"
#include <spdlog/spdlog.h>
#include <unordered_map>

namespace RenderStar::Client::Render::Vulkan
{
    struct VulkanUniformBuffer
    {
        VulkanBuffer buffer;
        uint32_t binding;
        size_t size;
    };

    class VulkanUniformManager : public IUniformManager
    {
    public:

        VulkanUniformManager();
        ~VulkanUniformManager() override;

        void Initialize(
            VulkanBufferModule* bufferModule,
            VulkanDescriptorModule* descriptorModule,
            uint32_t maxFramesInFlight);

        void Destroy();

        void CreateUniformBuffer(
            const std::string& name,
            const UniformBinding& binding) override;

        void UpdateUniformBuffer(
            const std::string& name,
            const void* data,
            size_t size,
            size_t offset = 0) override;

        void BindUniformBuffer(const std::string& name, uint32_t bindingPoint) override;

        IBufferHandle* GetUniformBuffer(const std::string& name) override;

        void DestroyUniformBuffer(const std::string& name) override;

        std::unique_ptr<IUniformBindingHandle> CreateBindingForShader(IShaderProgram* shader) override;

        VkBuffer GetVulkanBuffer(const std::string& name) const;
        VkDescriptorSet GetDescriptorSet(uint32_t frameIndex) const;

        void SetupDescriptorSets(VkDescriptorPool pool, VkDescriptorSetLayout layout);

    private:

        std::shared_ptr<spdlog::logger> logger;
        VulkanBufferModule* bufferModule;
        VulkanDescriptorModule* descriptorModule;
        uint32_t maxFramesInFlight;
        std::unordered_map<std::string, VulkanUniformBuffer> uniformBuffers;
        std::vector<VkDescriptorSet> descriptorSets;
        std::vector<VkDescriptorPool> createdPools;
    };
}
