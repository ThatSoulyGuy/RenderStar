#pragma once

#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanShaderModule.hpp"
#include <spdlog/spdlog.h>
#include <unordered_map>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanDescriptorModule;

    class VulkanShaderProgram : public IShaderProgram
    {
    public:

        VulkanShaderProgram();
        ~VulkanShaderProgram() override;

        void Initialize(
            VkDevice device,
            VkRenderPass renderPass,
            VulkanShaderModule* shaderModule,
            VulkanDescriptorModule* descriptorModule,
            VulkanShader vertexShader,
            VulkanShader fragmentShader,
            const VertexLayout& vertexLayout,
            IGraphicsResourceManager& resourceManager);

        void InitializeWithLayout(
            VkDevice device,
            VkRenderPass renderPass,
            VulkanShaderModule* shaderModule,
            VulkanShader vertexShader,
            VulkanShader fragmentShader,
            const VertexLayout& vertexLayout,
            VkDescriptorSetLayout externalLayout,
            IGraphicsResourceManager& resourceManager);

        void InitializeCompute(
            VulkanShaderModule* shaderModule,
            VulkanShader computeShader,
            IGraphicsResourceManager& resourceManager);

        void Release() override;
        GraphicsResourceType GetResourceType() const override;

        void BindPipeline(VkCommandBuffer commandBuffer);
        void BindDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet);

        bool IsValid() const override;

        const VulkanShader& GetVertexShader() const;
        const VulkanShader& GetFragmentShader() const;
        const VulkanShader& GetComputeShader() const;

        std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages() const;

        VkPipeline GetPipeline() const;
        VkPipelineLayout GetPipelineLayout() const;
        VkDescriptorSetLayout GetDescriptorSetLayout() const;

    private:

        void BuildPipeline(VkRenderPass renderPass, const VertexLayout& vertexLayout);
        void BuildPipelineWithLayout(VkRenderPass renderPass, const VertexLayout& vertexLayout, VkDescriptorSetLayout layout);
        void DestroyPipeline();

        VkFormat GetVulkanFormat(VertexAttributeType type) const;

        std::shared_ptr<spdlog::logger> logger;
        VkDevice device;
        VulkanShaderModule* shaderModule;
        VulkanDescriptorModule* descriptorModule;
        VulkanShader vertexShader;
        VulkanShader fragmentShader;
        VulkanShader computeShader;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        VkDescriptorSetLayout descriptorSetLayout;
        bool isCompute;
        bool valid;
    };
}
