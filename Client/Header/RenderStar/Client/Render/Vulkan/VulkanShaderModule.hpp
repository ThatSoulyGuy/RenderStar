#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <vector>
#include <string>
#include <unordered_map>

namespace RenderStar::Client::Render::Vulkan
{
    enum class VulkanShaderStage
    {
        VERTEX,
        FRAGMENT,
        COMPUTE
    };

    struct VulkanShader
    {
        VkShaderModule module;
        VulkanShaderStage stage;
    };

    struct VulkanPipeline
    {
        VkPipeline pipeline;
        VkPipelineLayout layout;
    };

    class VulkanShaderModule
    {
    public:

        VulkanShaderModule();
        ~VulkanShaderModule();

        void Create(VkDevice device);
        void Destroy();

        VulkanShader LoadShaderFromSpirv(const std::vector<uint32_t>& spirvCode, VulkanShaderStage stage);
        VulkanShader LoadShaderFromGlsl(const std::string& glslSource, VulkanShaderStage stage, const std::string& filename);
        VulkanShader LoadShaderFromFile(const std::string& filePath, VulkanShaderStage stage);

        void DestroyShader(VulkanShader& shader);

        VkPipelineShaderStageCreateInfo GetShaderStageInfo(const VulkanShader& shader) const;

    private:

        std::vector<uint32_t> CompileGlslToSpirv(const std::string& glslSource, VulkanShaderStage stage, const std::string& filename);
        VkShaderStageFlagBits GetVkShaderStage(VulkanShaderStage stage) const;

        std::shared_ptr<spdlog::logger> logger;
        VkDevice device;
    };
}
