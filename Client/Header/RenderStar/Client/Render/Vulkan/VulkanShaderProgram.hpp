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
            const VertexLayout& vertexLayout);

        void InitializeCompute(
            VulkanShaderModule* shaderModule,
            VulkanShader computeShader);

        void Bind() override;
        void Unbind() override;

        void BindPipeline(VkCommandBuffer commandBuffer);
        void BindDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet);

        bool IsValid() const override;

        void SetUniform(const std::string& name, int32_t value) override;
        void SetUniform(const std::string& name, float value) override;
        void SetUniform(const std::string& name, const glm::vec2& value) override;
        void SetUniform(const std::string& name, const glm::vec3& value) override;
        void SetUniform(const std::string& name, const glm::vec4& value) override;
        void SetUniform(const std::string& name, const glm::mat3& value) override;
        void SetUniform(const std::string& name, const glm::mat4& value) override;

        int32_t GetUniformLocation(const std::string& name) const override;

        const VulkanShader& GetVertexShader() const;
        const VulkanShader& GetFragmentShader() const;
        const VulkanShader& GetComputeShader() const;

        std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages() const;

        VkPipeline GetPipeline() const;
        VkPipelineLayout GetPipelineLayout() const;
        VkDescriptorSetLayout GetDescriptorSetLayout() const;

    private:

        void BuildPipeline(VkRenderPass renderPass, const VertexLayout& vertexLayout);
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
