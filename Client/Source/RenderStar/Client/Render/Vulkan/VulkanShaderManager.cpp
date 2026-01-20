#include "RenderStar/Client/Render/Vulkan/VulkanShaderManager.hpp"

#include "RenderStar/Client/Render/Vulkan/VulkanDescriptorModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanShaderProgram.hpp"
#include "RenderStar/Common/Asset/AssetLocation.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    VulkanShaderManager::VulkanShaderManager()
        : logger(spdlog::default_logger()->clone("VulkanShaderManager"))
        , device(VK_NULL_HANDLE)
        , renderPass(VK_NULL_HANDLE)
        , shaderModule(nullptr)
        , descriptorModule(nullptr)
        , vertexLayout{}
    {
    }

    VulkanShaderManager::~VulkanShaderManager() = default;

    void VulkanShaderManager::Initialize(
        VkDevice vulkanDevice,
        VkRenderPass vulkanRenderPass,
        VulkanShaderModule* module,
        VulkanDescriptorModule* descModule,
        const VertexLayout& defaultVertexLayout)
    {
        device = vulkanDevice;
        renderPass = vulkanRenderPass;
        shaderModule = module;
        descriptorModule = descModule;
        vertexLayout = defaultVertexLayout;
        logger->info("Vulkan shader manager initialized");
    }

    std::unique_ptr<IShaderProgram> VulkanShaderManager::CreateFromSource(const ShaderSource& source)
    {
        (void)source;
        logger->warn("CreateFromSource not implemented for Vulkan - use SPIR-V binary");
        return nullptr;
    }

    std::unique_ptr<IShaderProgram> VulkanShaderManager::CreateFromBinary(const ShaderBinary& binary)
    {
        auto program = std::make_unique<VulkanShaderProgram>();

        if (!binary.computeSpirv.empty())
        {
            VulkanShader computeShader = shaderModule->LoadShaderFromSpirv(
                binary.computeSpirv, VulkanShaderStage::COMPUTE);

            program->InitializeCompute(shaderModule, computeShader);
        }
        else
        {
            VulkanShader vertexShader = shaderModule->LoadShaderFromSpirv(
                binary.vertexSpirv, VulkanShaderStage::VERTEX);

            VulkanShader fragmentShader = shaderModule->LoadShaderFromSpirv(
                binary.fragmentSpirv, VulkanShaderStage::FRAGMENT);

            program->Initialize(device, renderPass, shaderModule, descriptorModule,
                vertexShader, fragmentShader, vertexLayout);
        }

        return program;
    }

    std::unique_ptr<IShaderProgram> VulkanShaderManager::LoadFromFile(const Common::Asset::AssetModule& assetModule, const Common::Asset::AssetLocation& vertexPath, const Common::Asset::AssetLocation& fragmentPath)
    {
        auto program = std::make_unique<VulkanShaderProgram>();

        const VulkanShader vertexShader = shaderModule->LoadShaderFromFile(vertexPath.ToFilesystemPath(assetModule.GetBasePath()).string(), VulkanShaderStage::VERTEX);

        const VulkanShader fragmentShader = shaderModule->LoadShaderFromFile(fragmentPath.ToFilesystemPath(assetModule.GetBasePath()).string(), VulkanShaderStage::FRAGMENT);

        program->Initialize(device, renderPass, shaderModule, descriptorModule, vertexShader, fragmentShader, vertexLayout);

        logger->info("Loaded shader from files: {}, {}", vertexPath.ToFilesystemPath(assetModule.GetBasePath()).string(), fragmentPath.ToFilesystemPath(assetModule.GetBasePath()).string());

        return program;
    }

    std::unique_ptr<IShaderProgram> VulkanShaderManager::LoadComputeFromFile(const std::string& computePath)
    {
        auto program = std::make_unique<VulkanShaderProgram>();

        const VulkanShader computeShader = shaderModule->LoadShaderFromFile(computePath, VulkanShaderStage::COMPUTE);

        program->InitializeCompute(shaderModule, computeShader);

        logger->info("Loaded compute shader from file: {}", computePath);

        return program;
    }

    void VulkanShaderManager::DestroyShader(IShaderProgram* shader)
    {
        (void)shader;
    }
}
