#include "RenderStar/Client/Render/Vulkan/VulkanUniformManager.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanShaderProgram.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanUniformBinding.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    VulkanUniformManager::VulkanUniformManager()
        : logger(spdlog::default_logger()->clone("VulkanUniformManager"))
        , bufferModule(nullptr)
        , descriptorModule(nullptr)
        , maxFramesInFlight(2)
    {
    }

    VulkanUniformManager::~VulkanUniformManager()
    {
        Destroy();
    }

    void VulkanUniformManager::Initialize(
        VulkanBufferModule* buffer,
        VulkanDescriptorModule* descriptor,
        uint32_t framesInFlight)
    {
        bufferModule = buffer;
        descriptorModule = descriptor;
        maxFramesInFlight = framesInFlight;
        logger->info("Vulkan uniform manager initialized");
    }

    void VulkanUniformManager::Destroy()
    {
        for (auto& [name, uniformBuffer] : uniformBuffers)
            bufferModule->DestroyBuffer(uniformBuffer.buffer);

        uniformBuffers.clear();
        descriptorSets.clear();

        for (VkDescriptorPool pool : createdPools)
            descriptorModule->DestroyDescriptorPool(pool);

        createdPools.clear();

        logger->info("Vulkan uniform manager destroyed");
    }

    void VulkanUniformManager::CreateUniformBuffer(
        const std::string& name,
        const UniformBinding& binding)
    {
        VulkanUniformBuffer uniformBuffer{};
        uniformBuffer.binding = binding.binding;
        uniformBuffer.size = binding.size;
        uniformBuffer.buffer = bufferModule->CreateBuffer(
            VulkanBufferType::UNIFORM, binding.size, true);

        uniformBuffers[name] = uniformBuffer;

        logger->info("Created uniform buffer: {} (binding: {}, size: {})",
            name, binding.binding, binding.size);
    }

    void VulkanUniformManager::UpdateUniformBuffer(
        const std::string& name,
        const void* data,
        size_t size,
        size_t offset)
    {
        auto it = uniformBuffers.find(name);

        if (it == uniformBuffers.end())
        {
            logger->warn("Uniform buffer not found: {}", name);
            return;
        }

        bufferModule->UploadBufferData(it->second.buffer, data, size, offset);
    }

    void VulkanUniformManager::BindUniformBuffer(const std::string& name, uint32_t bindingPoint)
    {
        (void)name;
        (void)bindingPoint;
    }

    IBufferHandle* VulkanUniformManager::GetUniformBuffer(const std::string& name)
    {
        (void)name;
        return nullptr;
    }

    void VulkanUniformManager::DestroyUniformBuffer(const std::string& name)
    {
        auto it = uniformBuffers.find(name);

        if (it != uniformBuffers.end())
        {
            bufferModule->DestroyBuffer(it->second.buffer);
            uniformBuffers.erase(it);
        }
    }

    VkBuffer VulkanUniformManager::GetVulkanBuffer(const std::string& name) const
    {
        auto it = uniformBuffers.find(name);

        if (it == uniformBuffers.end())
            return VK_NULL_HANDLE;

        return it->second.buffer.buffer;
    }

    VkDescriptorSet VulkanUniformManager::GetDescriptorSet(uint32_t frameIndex) const
    {
        if (frameIndex >= descriptorSets.size())
            return VK_NULL_HANDLE;

        return descriptorSets[frameIndex];
    }

    void VulkanUniformManager::SetupDescriptorSets(VkDescriptorPool pool, VkDescriptorSetLayout layout)
    {
        descriptorSets = descriptorModule->AllocateDescriptorSets(pool, layout, maxFramesInFlight);

        for (uint32_t i = 0; i < maxFramesInFlight; ++i)
        {
            for (const auto& [name, uniformBuffer] : uniformBuffers)
            {
                descriptorModule->UpdateDescriptorSetBuffer(
                    descriptorSets[i],
                    uniformBuffer.binding,
                    uniformBuffer.buffer.buffer,
                    0,
                    uniformBuffer.size);
            }
        }

        logger->info("Setup {} descriptor sets", maxFramesInFlight);
    }

    std::unique_ptr<IUniformBindingHandle> VulkanUniformManager::CreateBindingForShader(IShaderProgram* shader)
    {
        auto* vulkanShader = static_cast<VulkanShaderProgram*>(shader);

        if (vulkanShader == nullptr || !vulkanShader->IsValid())
        {
            logger->error("Invalid shader program for uniform binding");
            return nullptr;
        }

        VkDescriptorSetLayout layout = vulkanShader->GetDescriptorSetLayout();

        std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxFramesInFlight }
        };

        VkDescriptorPool pool = descriptorModule->CreateDescriptorPool(poolSizes, maxFramesInFlight);
        createdPools.push_back(pool);

        std::vector<VkDescriptorSet> sets = descriptorModule->AllocateDescriptorSets(pool, layout, maxFramesInFlight);

        logger->info("Created uniform binding with {} descriptor sets", maxFramesInFlight);

        return std::make_unique<VulkanUniformBinding>(sets, descriptorModule);
    }
}
