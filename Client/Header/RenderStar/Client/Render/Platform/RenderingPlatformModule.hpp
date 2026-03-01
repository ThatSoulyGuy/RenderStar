#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Client/Render/Platform/IRenderPlatformStage.hpp"
#include "RenderStar/Client/Render/Platform/IRenderTarget.hpp"
#include "RenderStar/Client/Render/Platform/IRenderingPlatformBackend.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace RenderStar::Client::Render
{
    class IRenderBackend;
}

namespace RenderStar::Client::Render::Platform
{
    class RenderingPlatformModule final : public Common::Module::AbstractModule
    {
    public:

        RenderingPlatformModule();
        ~RenderingPlatformModule() override;

        void Execute(IRenderBackend* backend);

        IRenderTarget* GetTarget(const std::string& name) const;

        void BeginTarget(const std::string& name, bool clear = true);
        void EndTarget(const std::string& name);
        ITextureHandle* GetTargetColorTexture(const std::string& name) const;

        std::unique_ptr<IShaderProgram> CompileShaderForTarget(
            const std::string& vertexGlsl,
            const std::string& fragmentGlsl,
            const std::string& targetName,
            const VertexLayout& vertexLayout);

        bool IsFrozen() const;

        bool IsEnabled() const;

        IShaderProgram* GetGeometryShader() const;

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;
        void OnCleanup() override;

    private:

        void LoadConfiguration(Common::Module::ModuleContext& context);
        void CreateRenderTargets();
        void CreateStages(Common::Module::ModuleContext& context);
        void Freeze();

        std::unique_ptr<IRenderingPlatformBackend> platformBackend;
        std::vector<std::unique_ptr<IRenderPlatformStage>> stages;
        std::unordered_map<std::string, std::unique_ptr<IRenderTarget>> renderTargets;
        std::unordered_map<std::string, IRenderTarget*> targetLookup;
        bool frozen = false;
        bool enabled = false;

        struct TargetConfig
        {
            std::string name;
            TextureFormat format = TextureFormat::RGBA8;
            bool hasDepth = false;
            bool matchSwapchain = true;
            uint32_t width = 0;
            uint32_t height = 0;
        };

        struct StageConfig
        {
            std::string name;
            std::string type;
            std::string shader;
            std::string output;
            std::string vertexFormat;
            bool clear = true;
            std::vector<std::string> inputs;
        };

        std::vector<TargetConfig> targetConfigs;
        std::vector<StageConfig> stageConfigs;
    };
}
