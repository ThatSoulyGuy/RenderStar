#pragma once

#include "RenderStar/Client/Render/Resource/IShaderManager.hpp"
#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanShaderModule.hpp"
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanDescriptorModule;

    class VulkanShaderManager : public IShaderManager
    {
    public:

        VulkanShaderManager();
        ~VulkanShaderManager() override;

        void Initialize(
            VkDevice device,
            VkRenderPass renderPass,
            VulkanShaderModule* shaderModule,
            VulkanDescriptorModule* descriptorModule,
            const VertexLayout& defaultVertexLayout);

        std::unique_ptr<IShaderProgram> CreateFromSource(const ShaderSource& source) override;
        std::unique_ptr<IShaderProgram> CreateFromBinary(const ShaderBinary& binary) override;

        std::unique_ptr<IShaderProgram> LoadFromFile(const Common::Asset::AssetModule&, const Common::Asset::AssetLocation&, const Common::Asset::AssetLocation&) override;

        std::unique_ptr<IShaderProgram> LoadComputeFromFile(const std::string& computePath) override;

        void DestroyShader(IShaderProgram* shader) override;

    private:

        std::shared_ptr<spdlog::logger> logger;
        VkDevice device;
        VkRenderPass renderPass;
        VulkanShaderModule* shaderModule;
        VulkanDescriptorModule* descriptorModule;
        VertexLayout vertexLayout;
    };
}
