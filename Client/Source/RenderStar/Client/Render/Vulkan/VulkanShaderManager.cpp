#include "RenderStar/Client/Render/Vulkan/VulkanShaderManager.hpp"

#include "RenderStar/Client/Render/Vulkan/VulkanDescriptorModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanShaderProgram.hpp"
#include "RenderStar/Common/Asset/ITextAsset.hpp"
#include "RenderStar/Common/Asset/IBinaryAsset.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    namespace
    {
        std::vector<uint32_t> ConvertToSpirv(const Common::Asset::IBinaryAsset& asset)
        {
            const auto& data = asset.GetData();
            size_t wordCount = data.size() / sizeof(uint32_t);
            std::vector<uint32_t> spirv(wordCount);
            std::memcpy(spirv.data(), data.data(), wordCount * sizeof(uint32_t));
            return spirv;
        }
    }

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
        auto program = std::make_unique<VulkanShaderProgram>();

        VulkanShader vertexShader = shaderModule->LoadShaderFromGlsl(source.vertexSource, VulkanShaderStage::VERTEX, "vertex");
        VulkanShader fragmentShader = shaderModule->LoadShaderFromGlsl(source.fragmentSource, VulkanShaderStage::FRAGMENT, "fragment");

        program->Initialize(device, renderPass, shaderModule, descriptorModule, vertexShader, fragmentShader, vertexLayout);

        logger->info("Created shader from GLSL source");

        return program;
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

    std::unique_ptr<IShaderProgram> VulkanShaderManager::CreateFromTextAssets(const Common::Asset::ITextAsset& vertexAsset, const Common::Asset::ITextAsset& fragmentAsset)
    {
        auto program = std::make_unique<VulkanShaderProgram>();

        VulkanShader vertexShader = shaderModule->LoadShaderFromGlsl(
            vertexAsset.GetContent(), VulkanShaderStage::VERTEX, vertexAsset.GetLocation().ToString());
        VulkanShader fragmentShader = shaderModule->LoadShaderFromGlsl(
            fragmentAsset.GetContent(), VulkanShaderStage::FRAGMENT, fragmentAsset.GetLocation().ToString());

        program->Initialize(device, renderPass, shaderModule, descriptorModule, vertexShader, fragmentShader, vertexLayout);

        logger->info("Created shader from text assets: {}, {}", vertexAsset.GetLocation().ToString(), fragmentAsset.GetLocation().ToString());

        return program;
    }

    std::unique_ptr<IShaderProgram> VulkanShaderManager::CreateFromBinaryAssets(const Common::Asset::IBinaryAsset& vertexAsset, const Common::Asset::IBinaryAsset& fragmentAsset)
    {
        auto program = std::make_unique<VulkanShaderProgram>();

        std::vector<uint32_t> vertexSpirv = ConvertToSpirv(vertexAsset);
        std::vector<uint32_t> fragmentSpirv = ConvertToSpirv(fragmentAsset);

        VulkanShader vertexShader = shaderModule->LoadShaderFromSpirv(vertexSpirv, VulkanShaderStage::VERTEX);
        VulkanShader fragmentShader = shaderModule->LoadShaderFromSpirv(fragmentSpirv, VulkanShaderStage::FRAGMENT);

        program->Initialize(device, renderPass, shaderModule, descriptorModule, vertexShader, fragmentShader, vertexLayout);

        logger->info("Loaded shader from binary assets: {}, {}", vertexAsset.GetLocation().ToString(), fragmentAsset.GetLocation().ToString());

        return program;
    }

    std::unique_ptr<IShaderProgram> VulkanShaderManager::CreateComputeFromTextAsset(const Common::Asset::ITextAsset& computeAsset)
    {
        auto program = std::make_unique<VulkanShaderProgram>();

        VulkanShader computeShader = shaderModule->LoadShaderFromGlsl(
            computeAsset.GetContent(), VulkanShaderStage::COMPUTE, computeAsset.GetLocation().ToString());

        program->InitializeCompute(shaderModule, computeShader);

        logger->info("Created compute shader from text asset: {}", computeAsset.GetLocation().ToString());

        return program;
    }

    std::unique_ptr<IShaderProgram> VulkanShaderManager::CreateComputeFromBinaryAsset(const Common::Asset::IBinaryAsset& computeAsset)
    {
        auto program = std::make_unique<VulkanShaderProgram>();

        std::vector<uint32_t> computeSpirv = ConvertToSpirv(computeAsset);
        VulkanShader computeShader = shaderModule->LoadShaderFromSpirv(computeSpirv, VulkanShaderStage::COMPUTE);

        program->InitializeCompute(shaderModule, computeShader);

        logger->info("Loaded compute shader from binary asset: {}", computeAsset.GetLocation().ToString());

        return program;
    }

    void VulkanShaderManager::DestroyShader(IShaderProgram* shader)
    {
        (void)shader;
    }
}
