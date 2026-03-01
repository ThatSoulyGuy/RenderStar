#include "RenderStar/Client/Render/Vulkan/VulkanPlatformBackend.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanRenderBackend.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanRenderTarget.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanShaderProgram.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanShaderModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanDescriptorModule.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanCommandModule.hpp"
#include "RenderStar/Client/Render/Resource/Vertex.hpp"
#include "RenderStar/Client/Render/Resource/IGraphicsResourceManager.hpp"
#include <regex>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanPlatformBackend::VulkanPlatformBackend(VulkanRenderBackend* backend)
        : logger(spdlog::default_logger()->clone("VulkanPlatformBackend"))
        , backend(backend)
        , shaderModule(&backend->GetShaderModuleRef())
        , descriptorModule(&backend->GetDescriptorModuleRef())
        , commandModule(&backend->GetCommandModuleRef())
    {
        logger->info("Vulkan platform backend created");
    }

    VulkanPlatformBackend::~VulkanPlatformBackend()
    {
        VkDevice device = backend->GetDevice();

        for (auto& [shader, cache] : descriptorCaches)
        {
            if (cache.pool != VK_NULL_HANDLE)
                descriptorModule->DestroyDescriptorPool(cache.pool);
        }

        descriptorCaches.clear();

        for (auto layout : ownedLayouts)
        {
            VulkanDescriptorSetLayout wrapper;
            wrapper.layout = layout;
            descriptorModule->DestroyDescriptorSetLayout(wrapper);
        }

        ownedLayouts.clear();
    }

    std::unique_ptr<Platform::IRenderTarget> VulkanPlatformBackend::CreateRenderTarget(
        const Platform::RenderTargetDescription& description)
    {
        auto desc = description;

        if (desc.matchSwapchainSize)
        {
            desc.width = backend->GetWidth();
            desc.height = backend->GetHeight();
        }

        return std::make_unique<VulkanRenderTarget>(
            backend->GetDevice(),
            backend->GetPhysicalDevice(),
            backend->GetAllocator(),
            desc);
    }

    std::unique_ptr<Platform::IRenderTarget> VulkanPlatformBackend::CreateSwapchainTarget()
    {
        return std::make_unique<VulkanSwapchainTarget>(
            backend->GetWidth(),
            backend->GetHeight(),
            backend->GetSwapchainRenderPass());
    }

    std::unique_ptr<IShaderProgram> VulkanPlatformBackend::CompileShader(
        const std::string& vertexGlsl,
        const std::string& fragmentGlsl)
    {
        return CompileShaderForTarget(vertexGlsl, fragmentGlsl, nullptr, Vertex::LAYOUT);
    }

    VkDescriptorSetLayout VulkanPlatformBackend::CreateDescriptorLayoutFromGlsl(
        const std::string& vertexGlsl,
        const std::string& fragmentGlsl)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        std::regex uniformRegex(R"(layout\s*\(\s*binding\s*=\s*(\d+)\s*\)\s*uniform\s+(?!sampler)(\w+))");
        std::regex samplerRegex(R"(layout\s*\(\s*binding\s*=\s*(\d+)\s*\)\s*uniform\s+sampler\w*\s+(\w+))");

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

    std::unique_ptr<IShaderProgram> VulkanPlatformBackend::CompileShaderForTarget(
        const std::string& vertexGlsl,
        const std::string& fragmentGlsl,
        Platform::IRenderTarget* target,
        const VertexLayout& vertexLayout)
    {
        VkRenderPass renderPass = target ? GetRenderPassForTarget(target) : backend->GetSwapchainRenderPass();

        VulkanShader vertexShader = shaderModule->LoadShaderFromGlsl(
            vertexGlsl, VulkanShaderStage::VERTEX, "platform_vertex");
        VulkanShader fragmentShader = shaderModule->LoadShaderFromGlsl(
            fragmentGlsl, VulkanShaderStage::FRAGMENT, "platform_fragment");

        if (vertexShader.module == VK_NULL_HANDLE || fragmentShader.module == VK_NULL_HANDLE)
        {
            logger->error("Failed to compile platform shader modules");

            if (vertexShader.module != VK_NULL_HANDLE)
                shaderModule->DestroyShader(vertexShader);

            if (fragmentShader.module != VK_NULL_HANDLE)
                shaderModule->DestroyShader(fragmentShader);

            return nullptr;
        }

        VkDescriptorSetLayout layout = CreateDescriptorLayoutFromGlsl(vertexGlsl, fragmentGlsl);

        auto program = std::make_unique<VulkanShaderProgram>();
        program->InitializeWithLayout(
            backend->GetDevice(),
            renderPass,
            shaderModule,
            vertexShader,
            fragmentShader,
            vertexLayout,
            layout,
            backend->GetResourceManagerRef());

        return program;
    }

    std::unique_ptr<IShaderProgram> VulkanPlatformBackend::CompileComputeShader(
        const std::string& computeGlsl)
    {
        VulkanShader computeShader = shaderModule->LoadShaderFromGlsl(
            computeGlsl, VulkanShaderStage::COMPUTE, "platform_compute");

        if (computeShader.module == VK_NULL_HANDLE)
        {
            logger->error("Failed to compile compute shader module");
            return nullptr;
        }

        auto program = std::make_unique<VulkanShaderProgram>();
        program->InitializeCompute(shaderModule, computeShader, backend->GetResourceManagerRef());

        return program;
    }

    VkRenderPass VulkanPlatformBackend::GetRenderPassForTarget(Platform::IRenderTarget* target)
    {
        if (target->IsSwapchain())
        {
            auto* swapchainTarget = static_cast<VulkanSwapchainTarget*>(target);
            return swapchainTarget->GetRenderPass();
        }

        auto* vulkanTarget = static_cast<VulkanRenderTarget*>(target);
        return vulkanTarget->GetRenderPass();
    }

    VkFramebuffer VulkanPlatformBackend::GetFramebufferForTarget(Platform::IRenderTarget* target)
    {
        if (target->IsSwapchain())
            return backend->GetSwapchainFramebuffer(backend->GetCurrentImageIndex());

        auto* vulkanTarget = static_cast<VulkanRenderTarget*>(target);
        return vulkanTarget->GetFramebuffer();
    }

    PlatformDescriptorCache& VulkanPlatformBackend::GetOrCreateDescriptorCache(IShaderProgram* shader)
    {
        auto it = descriptorCaches.find(shader);

        if (it != descriptorCaches.end())
            return it->second;

        auto* vulkanShader = static_cast<VulkanShaderProgram*>(shader);
        VkDescriptorSetLayout layout = vulkanShader->GetDescriptorSetLayout();

        std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(backend->GetMaxFramesInFlight() * 3) },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(backend->GetMaxFramesInFlight() * 10) }
        };

        VkDescriptorPool pool = descriptorModule->CreateDescriptorPool(
            poolSizes, static_cast<uint32_t>(backend->GetMaxFramesInFlight()));

        auto sets = descriptorModule->AllocateDescriptorSets(
            pool, layout, static_cast<uint32_t>(backend->GetMaxFramesInFlight()));

        PlatformDescriptorCache cache;
        cache.pool = pool;
        cache.sets = std::move(sets);

        auto [insertIt, _] = descriptorCaches.emplace(shader, std::move(cache));
        return insertIt->second;
    }

    void VulkanPlatformBackend::BindInputTextures(
        const std::vector<Platform::IRenderTarget*>& inputs,
        IShaderProgram* shader,
        int32_t frameIndex)
    {
        if (inputs.empty() || !shader)
            return;

        auto& cache = GetOrCreateDescriptorCache(shader);

        if (frameIndex < 0 || frameIndex >= static_cast<int32_t>(cache.sets.size()))
            return;

        VkDescriptorSet descriptorSet = cache.sets[frameIndex];

        for (uint32_t i = 0; i < inputs.size(); i++)
        {
            auto* colorAttachment = inputs[i]->GetColorAttachment(0);

            if (!colorAttachment)
                continue;

            auto* attachment = dynamic_cast<VulkanRenderTargetAttachment*>(colorAttachment);

            if (!attachment)
                continue;

            descriptorModule->UpdateDescriptorSetImage(
                descriptorSet,
                i,
                attachment->GetImageView(),
                attachment->GetSampler());
        }

        auto* vulkanShader = static_cast<VulkanShaderProgram*>(shader);
        VkCommandBuffer cmd = commandModule->GetCurrentCommandBuffer();
        vulkanShader->BindDescriptorSet(cmd, descriptorSet);
    }

    void VulkanPlatformBackend::BeginRenderTarget(Platform::IRenderTarget* target, bool clear)
    {
        commandModule->EndRenderPass();

        VkRenderPass renderPass = GetRenderPassForTarget(target);
        VkFramebuffer framebuffer = GetFramebufferForTarget(target);

        commandModule->BeginRenderPass(
            renderPass,
            framebuffer,
            static_cast<int32_t>(target->GetWidth()),
            static_cast<int32_t>(target->GetHeight()));

        if (target->IsSwapchain())
        {
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(target->GetWidth());
            viewport.height = static_cast<float>(target->GetHeight());
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkCommandBuffer cmd = commandModule->GetCurrentCommandBuffer();
            vkCmdSetViewport(cmd, 0, 1, &viewport);
        }
    }

    void VulkanPlatformBackend::EndRenderTarget(Platform::IRenderTarget*)
    {
        commandModule->EndRenderPass();
    }

    void VulkanPlatformBackend::BlitToScreen(Platform::IRenderTarget*)
    {
    }

    IBufferManager* VulkanPlatformBackend::GetBufferManager()
    {
        return backend->GetBufferManager();
    }

    IUniformManager* VulkanPlatformBackend::GetUniformManager()
    {
        return backend->GetUniformManager();
    }

    ITextureManager* VulkanPlatformBackend::GetTextureManager()
    {
        return backend->GetTextureManager();
    }

    IRenderCommandQueue* VulkanPlatformBackend::GetCommandQueue()
    {
        return backend->GetCommandQueue();
    }

    void VulkanPlatformBackend::SubmitDrawCommand(
        IShaderProgram* shader,
        IUniformBindingHandle* uniformBinding,
        int32_t frameIndex,
        IMesh* mesh)
    {
        backend->SubmitDrawCommand(shader, uniformBinding, frameIndex, mesh);
    }

    void VulkanPlatformBackend::ExecuteDrawCommands()
    {
        backend->ExecuteDrawCommands();
    }

    void VulkanPlatformBackend::OnResize(uint32_t, uint32_t)
    {
    }

    uint32_t VulkanPlatformBackend::GetWidth() const
    {
        return backend->GetWidth();
    }

    uint32_t VulkanPlatformBackend::GetHeight() const
    {
        return backend->GetHeight();
    }

    int32_t VulkanPlatformBackend::GetCurrentFrame() const
    {
        return backend->GetCurrentFrame();
    }

    int32_t VulkanPlatformBackend::GetMaxFramesInFlight() const
    {
        return backend->GetMaxFramesInFlight();
    }
}
