#include "RenderStar/Client/Render/Vulkan/VulkanShaderManager.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanDescriptorModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IGraphicsResourceManager.hpp"
#include "RenderStar/Common/Asset/ITextAsset.hpp"
#include "RenderStar/Common/Asset/IBinaryAsset.hpp"
#include <regex>

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
        , resourceManager(nullptr)
    {
    }

    VulkanShaderManager::~VulkanShaderManager()
    {
        for (auto layout : ownedLayouts)
        {
            VulkanDescriptorSetLayout wrapper;
            wrapper.layout = layout;
            descriptorModule->DestroyDescriptorSetLayout(wrapper);
        }

        ownedLayouts.clear();
    }

    void VulkanShaderManager::Initialize(
        VkDevice vulkanDevice,
        VkRenderPass vulkanRenderPass,
        VulkanShaderModule* module,
        VulkanDescriptorModule* descModule,
        const VertexLayout& defaultVertexLayout,
        IGraphicsResourceManager* manager)
    {
        device = vulkanDevice;
        renderPass = vulkanRenderPass;
        shaderModule = module;
        descriptorModule = descModule;
        vertexLayout = defaultVertexLayout;
        resourceManager = manager;
        logger->info("Vulkan shader manager initialized");
    }

    std::unique_ptr<IShaderProgram> VulkanShaderManager::CreateFromSource(const ShaderSource& source)
    {
        auto program = std::make_unique<VulkanShaderProgram>();

        VulkanShader vertexShader = shaderModule->LoadShaderFromGlsl(source.vertexSource, VulkanShaderStage::VERTEX, "vertex");
        VulkanShader fragmentShader = shaderModule->LoadShaderFromGlsl(source.fragmentSource, VulkanShaderStage::FRAGMENT, "fragment");

        VkDescriptorSetLayout layout = CreateDescriptorLayoutFromGlsl(source.vertexSource, source.fragmentSource);

        program->InitializeWithLayout(device, renderPass, shaderModule, vertexShader, fragmentShader, vertexLayout, layout, *resourceManager);

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

            program->InitializeCompute(shaderModule, computeShader, *resourceManager);
        }
        else
        {
            VulkanShader vertexShader = shaderModule->LoadShaderFromSpirv(
                binary.vertexSpirv, VulkanShaderStage::VERTEX);

            VulkanShader fragmentShader = shaderModule->LoadShaderFromSpirv(
                binary.fragmentSpirv, VulkanShaderStage::FRAGMENT);

            program->Initialize(device, renderPass, shaderModule, descriptorModule,
                vertexShader, fragmentShader, vertexLayout, *resourceManager);
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

        program->Initialize(device, renderPass, shaderModule, descriptorModule, vertexShader, fragmentShader, vertexLayout, *resourceManager);

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

        program->Initialize(device, renderPass, shaderModule, descriptorModule, vertexShader, fragmentShader, vertexLayout, *resourceManager);

        logger->info("Loaded shader from binary assets: {}, {}", vertexAsset.GetLocation().ToString(), fragmentAsset.GetLocation().ToString());

        return program;
    }

    std::unique_ptr<IShaderProgram> VulkanShaderManager::CreateComputeFromTextAsset(const Common::Asset::ITextAsset& computeAsset)
    {
        auto program = std::make_unique<VulkanShaderProgram>();

        VulkanShader computeShader = shaderModule->LoadShaderFromGlsl(
            computeAsset.GetContent(), VulkanShaderStage::COMPUTE, computeAsset.GetLocation().ToString());

        program->InitializeCompute(shaderModule, computeShader, *resourceManager);

        logger->info("Created compute shader from text asset: {}", computeAsset.GetLocation().ToString());

        return program;
    }

    std::unique_ptr<IShaderProgram> VulkanShaderManager::CreateComputeFromBinaryAsset(const Common::Asset::IBinaryAsset& computeAsset)
    {
        auto program = std::make_unique<VulkanShaderProgram>();

        std::vector<uint32_t> computeSpirv = ConvertToSpirv(computeAsset);
        VulkanShader computeShader = shaderModule->LoadShaderFromSpirv(computeSpirv, VulkanShaderStage::COMPUTE);

        program->InitializeCompute(shaderModule, computeShader, *resourceManager);

        logger->info("Loaded compute shader from binary asset: {}", computeAsset.GetLocation().ToString());

        return program;
    }

    void VulkanShaderManager::DestroyShader(IShaderProgram* shader)
    {
        (void)shader;
    }

    VkDescriptorSetLayout VulkanShaderManager::CreateDescriptorLayoutFromGlsl(
        const std::string& vertexGlsl,
        const std::string& fragmentGlsl)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        std::regex uniformRegex(R"(layout\s*\([^)]*binding\s*=\s*(\d+)[^)]*\)\s*uniform\s+(?!sampler)(\w+))");
        std::regex samplerRegex(R"(layout\s*\([^)]*binding\s*=\s*(\d+)[^)]*\)\s*uniform\s+sampler\w*\s+(\w+))");

        auto addBindings = [&](const std::string& source, VkShaderStageFlags stageFlag)
        {
            std::sregex_iterator it(source.begin(), source.end(), uniformRegex);
            std::sregex_iterator end;

            for (; it != end; ++it)
            {
                uint32_t binding = std::stoul((*it)[1].str());

                bool found = false;
                for (auto& b : bindings)
                {
                    if (b.binding == binding && b.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                    {
                        b.stageFlags |= stageFlag;
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    VkDescriptorSetLayoutBinding b{};
                    b.binding = binding;
                    b.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    b.descriptorCount = 1;
                    b.stageFlags = stageFlag;
                    bindings.push_back(b);
                }
            }

            std::sregex_iterator sit(source.begin(), source.end(), samplerRegex);

            for (; sit != end; ++sit)
            {
                uint32_t binding = std::stoul((*sit)[1].str());

                bool found = false;
                for (auto& b : bindings)
                {
                    if (b.binding == binding && b.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                    {
                        b.stageFlags |= stageFlag;
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    VkDescriptorSetLayoutBinding b{};
                    b.binding = binding;
                    b.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    b.descriptorCount = 1;
                    b.stageFlags = stageFlag;
                    b.pImmutableSamplers = nullptr;
                    bindings.push_back(b);
                }
            }
        };

        addBindings(vertexGlsl, VK_SHADER_STAGE_VERTEX_BIT);
        addBindings(fragmentGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

        if (bindings.empty())
        {
            VkDescriptorSetLayoutBinding dummy{};
            dummy.binding = 0;
            dummy.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            dummy.descriptorCount = 1;
            dummy.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            bindings.push_back(dummy);
        }

        auto result = descriptorModule->CreateDescriptorSetLayout(bindings);
        ownedLayouts.push_back(result.layout);

        return result.layout;
    }
}
