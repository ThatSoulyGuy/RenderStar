#include "RenderStar/Client/Render/Platform/RenderingPlatformModule.hpp"
#include "RenderStar/Client/Render/Platform/PlatformBackendFactory.hpp"
#include "RenderStar/Client/Render/Platform/StageExecutionContext.hpp"
#include "RenderStar/Client/Render/Platform/GeometryStage.hpp"
#include "RenderStar/Client/Render/Platform/FullscreenStage.hpp"
#include "RenderStar/Client/Render/Platform/ComputeStage.hpp"
#include "RenderStar/Client/Render/RendererModule.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Shader/RsslCompiler.hpp"
#include "RenderStar/Client/Render/Resource/Vertex.hpp"
#include "RenderStar/Client/Render/Framework/LitVertex.hpp"
#include "RenderStar/Common/Configuration/ConfigurationModule.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"

namespace RenderStar::Client::Render::Platform
{
    namespace
    {
        TextureFormat ParseTextureFormat(const std::string& str)
        {
            if (str == "RGBA16F") return TextureFormat::RGBA16F;
            if (str == "RGBA32F") return TextureFormat::RGBA32F;
            return TextureFormat::RGBA8;
        }

        VertexLayout ResolveVertexFormat(const std::string& format)
        {
            if (format == "lit") return Framework::LitVertex::LAYOUT;
            if (format == "position_only") return VertexLayout::PositionOnly();
            return Vertex::LAYOUT;
        }
    }

    RenderingPlatformModule::RenderingPlatformModule() = default;

    RenderingPlatformModule::~RenderingPlatformModule() = default;

    void RenderingPlatformModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        LoadConfiguration(context);

        if (!enabled)
        {
            logger->info("RenderingPlatformModule disabled via config");
            return;
        }

        auto& rendererModule = context.GetDependency<RendererModule>();
        auto* backend = rendererModule.GetBackend();

        if (!backend || !backend->IsInitialized())
        {
            logger->warn("Render backend not available, disabling platform");
            enabled = false;
            return;
        }

        platformBackend = PlatformBackendFactory::Create(backend);

        if (!platformBackend)
        {
            logger->warn("No platform backend available for this render backend, disabling");
            enabled = false;
            return;
        }

        CreateRenderTargets();
        CreateStages(context);
        Freeze();
    }

    void RenderingPlatformModule::OnCleanup()
    {
        stages.clear();
        renderTargets.clear();
        targetLookup.clear();
        platformBackend.reset();
        frozen = false;
    }

    void RenderingPlatformModule::LoadConfiguration(Common::Module::ModuleContext& context)
    {
        auto& configModule = context.GetDependency<Common::Configuration::ConfigurationModule>();
        auto configOpt = configModule.For<RenderingPlatformModule>("render_star", "render_settings.xml");

        if (!configOpt)
        {
            logger->info("No render_settings.xml found, platform disabled");
            return;
        }

        auto& config = *configOpt;
        enabled = config->GetBoolean("enabled").value_or(false);

        if (!enabled)
            return;

        auto targetNames = config->GetStringList("target_names");

        for (const auto& name : targetNames)
        {
            TargetConfig tc;
            tc.name = name;

            auto formatStr = config->GetString("targets." + name + ".format").value_or("RGBA8");
            tc.format = ParseTextureFormat(formatStr);
            tc.hasDepth = config->GetBoolean("targets." + name + ".depth").value_or(false);
            tc.matchSwapchain = config->GetBoolean("targets." + name + ".match_swapchain").value_or(true);
            tc.width = static_cast<uint32_t>(config->GetInteger("targets." + name + ".width").value_or(0));
            tc.height = static_cast<uint32_t>(config->GetInteger("targets." + name + ".height").value_or(0));
            tc.sampleCount = static_cast<uint32_t>(config->GetInteger("targets." + name + ".msaa").value_or(1));

            targetConfigs.push_back(std::move(tc));
        }

        auto stageOrder = config->GetStringList("stage_order");

        for (const auto& name : stageOrder)
        {
            StageConfig sc;
            sc.name = name;
            sc.type = config->GetString("stages." + name + ".type").value_or("geometry");
            sc.shader = config->GetString("stages." + name + ".shader").value_or("");
            sc.output = config->GetString("stages." + name + ".output").value_or("SWAPCHAIN");
            sc.vertexFormat = config->GetString("stages." + name + ".vertex_format").value_or("standard");
            sc.clear = config->GetBoolean("stages." + name + ".clear").value_or(true);
            sc.inputs = config->GetStringList("stages." + name + ".inputs");

            stageConfigs.push_back(std::move(sc));
        }

        logger->info("Loaded config: {} targets, {} stages", targetConfigs.size(), stageConfigs.size());
    }

    void RenderingPlatformModule::CreateRenderTargets()
    {
        auto swapchain = platformBackend->CreateSwapchainTarget();
        targetLookup["SWAPCHAIN"] = swapchain.get();
        renderTargets["SWAPCHAIN"] = std::move(swapchain);

        for (const auto& tc : targetConfigs)
        {
            RenderTargetDescription desc;
            desc.name = tc.name;
            desc.colorFormat = tc.format;
            desc.hasDepth = tc.hasDepth;
            desc.matchSwapchainSize = tc.matchSwapchain;
            desc.sampleCount = tc.sampleCount;

            if (!tc.matchSwapchain && tc.width > 0 && tc.height > 0)
            {
                desc.width = tc.width;
                desc.height = tc.height;
            }

            auto target = platformBackend->CreateRenderTarget(desc);

            if (target)
            {
                logger->info("Created render target '{}' {}x{}", tc.name, target->GetWidth(), target->GetHeight());
                targetLookup[tc.name] = target.get();
                renderTargets[tc.name] = std::move(target);
            }
        }
    }

    void RenderingPlatformModule::CreateStages(Common::Module::ModuleContext& context)
    {
        auto& assetModule = context.GetDependency<Common::Asset::AssetModule>();

        for (const auto& sc : stageConfigs)
        {
            if (sc.shader.empty())
            {
                logger->error("Stage '{}' has no shader path", sc.name);
                continue;
            }

            auto location = Common::Asset::AssetLocation::Parse(sc.shader);
            auto asset = assetModule.LoadText(location);

            if (!asset)
            {
                logger->error("Stage '{}': failed to load RSSL from '{}'", sc.name, sc.shader);
                continue;
            }

            auto compileResult = Shader::RsslCompiler::Compile(asset->GetContent());

            if (!compileResult.IsValid())
            {
                for (const auto& error : compileResult.errors)
                    logger->error("Stage '{}' RSSL error: {}", sc.name, error);

                continue;
            }

            std::unique_ptr<IRenderPlatformStage> stage;

            if (sc.type == "geometry")
            {
                stage = std::make_unique<GeometryStage>(
                    sc.name, compileResult.vertexGlsl, compileResult.fragmentGlsl,
                    sc.output, sc.clear, ResolveVertexFormat(sc.vertexFormat));
            }
            else if (sc.type == "fullscreen")
            {
                stage = std::make_unique<FullscreenStage>(
                    sc.name, compileResult.vertexGlsl, compileResult.fragmentGlsl,
                    sc.inputs, sc.output);
            }
            else if (sc.type == "compute")
            {
                stage = std::make_unique<ComputeStage>(
                    sc.name, compileResult.computeGlsl,
                    sc.inputs, sc.output, 16, 16, 1);
            }
            else
            {
                logger->error("Stage '{}': unknown type '{}'", sc.name, sc.type);
                continue;
            }

            stage->Initialize(platformBackend.get(), targetLookup);
            stages.push_back(std::move(stage));
        }
    }

    void RenderingPlatformModule::Freeze()
    {
        frozen = true;
        logger->info("Rendering platform frozen: {} stages, {} targets", stages.size(), renderTargets.size());

        for (const auto& stage : stages)
            logger->info("  Stage '{}' -> '{}'", stage->GetName(), stage->GetOutputTargetName());
    }

    void RenderingPlatformModule::Execute(IRenderBackend* backend)
    {
        if (!frozen || !enabled || !platformBackend)
            return;

        uint32_t currentWidth = platformBackend->GetWidth();
        uint32_t currentHeight = platformBackend->GetHeight();

        auto* swapchainTarget = GetTarget("SWAPCHAIN");

        if (swapchainTarget && (swapchainTarget->GetWidth() != currentWidth || swapchainTarget->GetHeight() != currentHeight))
        {
            swapchainTarget->Resize(currentWidth, currentHeight);

            for (size_t i = 0; i < targetConfigs.size(); i++)
            {
                if (targetConfigs[i].matchSwapchain)
                {
                    auto* target = GetTarget(targetConfigs[i].name);

                    if (target)
                        target->Resize(currentWidth, currentHeight);
                }
            }
        }

        StageExecutionContext context(platformBackend.get(), targetLookup, backend->GetCurrentFrame());

        for (auto& stage : stages)
        {
            if (stage->IsEnabled())
                stage->Execute(context);
        }
    }

    IRenderTarget* RenderingPlatformModule::GetTarget(const std::string& name) const
    {
        auto it = targetLookup.find(name);

        if (it != targetLookup.end())
            return it->second;

        return nullptr;
    }

    void RenderingPlatformModule::BeginTarget(const std::string& name, bool clear)
    {
        if (!platformBackend)
            return;

        auto* target = GetTarget(name);

        if (target)
            platformBackend->BeginRenderTarget(target, clear);
    }

    void RenderingPlatformModule::EndTarget(const std::string& name)
    {
        if (!platformBackend)
            return;

        auto* target = GetTarget(name);

        if (target)
            platformBackend->EndRenderTarget(target);
    }

    ITextureHandle* RenderingPlatformModule::GetTargetColorTexture(const std::string& name) const
    {
        auto* target = GetTarget(name);

        if (target)
            return target->GetColorAttachment(0);

        return nullptr;
    }

    bool RenderingPlatformModule::IsFrozen() const
    {
        return frozen;
    }

    bool RenderingPlatformModule::IsEnabled() const
    {
        return enabled;
    }

    IShaderProgram* RenderingPlatformModule::GetGeometryShader() const
    {
        for (const auto& stage : stages)
        {
            auto* geometryStage = dynamic_cast<GeometryStage*>(stage.get());

            if (geometryStage)
                return geometryStage->GetShader();
        }

        return nullptr;
    }

    std::unique_ptr<IShaderProgram> RenderingPlatformModule::CompileShaderForTarget(
        const std::string& vertexGlsl,
        const std::string& fragmentGlsl,
        const std::string& targetName,
        const VertexLayout& vertexLayout)
    {
        if (!platformBackend)
            return nullptr;

        auto* target = GetTarget(targetName);

        if (!target)
        {
            logger->error("CompileShaderForTarget: target '{}' not found", targetName);
            return nullptr;
        }

        return platformBackend->CompileShaderForTarget(vertexGlsl, fragmentGlsl, target, vertexLayout);
    }

    std::vector<std::type_index> RenderingPlatformModule::GetDependencies() const
    {
        return DependsOn<
            RendererModule,
            Common::Configuration::ConfigurationModule,
            Common::Asset::AssetModule>();
    }
}
