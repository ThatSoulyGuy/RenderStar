#pragma once

#include "RenderStar/Client/Render/Platform/IRenderingPlatformBackend.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <unordered_map>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanRenderBackend;
    class VulkanShaderModule;
    class VulkanDescriptorModule;
    class VulkanCommandModule;

    struct PlatformDescriptorCache
    {
        VkDescriptorPool pool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> sets;
    };

    class VulkanPlatformBackend : public Platform::IRenderingPlatformBackend
    {
    public:

        explicit VulkanPlatformBackend(VulkanRenderBackend* backend);
        ~VulkanPlatformBackend() override;

        std::unique_ptr<Platform::IRenderTarget> CreateRenderTarget(
            const Platform::RenderTargetDescription& description) override;

        std::unique_ptr<Platform::IRenderTarget> CreateSwapchainTarget() override;

        std::unique_ptr<IShaderProgram> CompileShader(
            const std::string& vertexGlsl,
            const std::string& fragmentGlsl) override;

        std::unique_ptr<IShaderProgram> CompileShaderForTarget(
            const std::string& vertexGlsl,
            const std::string& fragmentGlsl,
            Platform::IRenderTarget* target,
            const VertexLayout& vertexLayout) override;

        std::unique_ptr<IShaderProgram> CompileComputeShader(
            const std::string& computeGlsl) override;

        void BindInputTextures(
            const std::vector<Platform::IRenderTarget*>& inputs,
            IShaderProgram* shader,
            int32_t frameIndex) override;

        void BeginRenderTarget(Platform::IRenderTarget* target, bool clear) override;
        void EndRenderTarget(Platform::IRenderTarget* target) override;
        void BlitToScreen(Platform::IRenderTarget* source) override;

        IBufferManager* GetBufferManager() override;
        IUniformManager* GetUniformManager() override;
        ITextureManager* GetTextureManager() override;
        IRenderCommandQueue* GetCommandQueue() override;

        void SubmitDrawCommand(
            IShaderProgram* shader,
            IUniformBindingHandle* uniformBinding,
            int32_t frameIndex,
            IMesh* mesh) override;

        void ExecuteDrawCommands() override;

        void OnResize(uint32_t width, uint32_t height) override;
        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;
        int32_t GetCurrentFrame() const override;
        int32_t GetMaxFramesInFlight() const override;

    private:

        VkDescriptorSetLayout CreateDescriptorLayoutFromGlsl(
            const std::string& vertexGlsl,
            const std::string& fragmentGlsl);

        VkRenderPass GetRenderPassForTarget(Platform::IRenderTarget* target);
        VkFramebuffer GetFramebufferForTarget(Platform::IRenderTarget* target);

        PlatformDescriptorCache& GetOrCreateDescriptorCache(IShaderProgram* shader);

        std::shared_ptr<spdlog::logger> logger;
        VulkanRenderBackend* backend;
        VulkanShaderModule* shaderModule;
        VulkanDescriptorModule* descriptorModule;
        VulkanCommandModule* commandModule;

        std::vector<VkDescriptorSetLayout> ownedLayouts;
        std::unordered_map<IShaderProgram*, PlatformDescriptorCache> descriptorCaches;
    };
}
