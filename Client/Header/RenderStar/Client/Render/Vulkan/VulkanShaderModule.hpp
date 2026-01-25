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

        [[nodiscard]]
        VulkanShader LoadShaderFromSpirv(const std::vector<uint32_t>& spirvCode, VulkanShaderStage stage) const;

        VulkanShader LoadShaderFromGlsl(const std::string& glslSource, VulkanShaderStage stage, const std::string& filename);
        VulkanShader LoadShaderFromFile(const std::string& filePath, VulkanShaderStage stage);

        void DestroyShader(VulkanShader& shader) const;
        static VkPipelineShaderStageCreateInfo GetShaderStageInfo(const VulkanShader& shader);

        static VkShaderStageFlagBits GetVkShaderStage(VulkanShaderStage stage);

    private:

        [[nodiscard]]
        std::vector<uint32_t> CompileGlslToSpirv(const std::string& glslSource, VulkanShaderStage stage, const std::string& filename) const;

        std::shared_ptr<spdlog::logger> logger;
        VkDevice device;
    };
}
