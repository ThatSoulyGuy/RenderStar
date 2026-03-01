#include "RenderStar/Client/Render/Platform/FullscreenStage.hpp"
#include "RenderStar/Client/Render/Platform/StageExecutionContext.hpp"
#include "RenderStar/Client/Render/Platform/IRenderingPlatformBackend.hpp"
#include "RenderStar/Client/Render/Platform/IRenderTarget.hpp"
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Platform
{
    FullscreenStage::FullscreenStage(
        const std::string& name,
        const std::string& rsslVertexGlsl,
        const std::string& rsslFragmentGlsl,
        const std::vector<std::string>& inputTargets,
        const std::string& outputTarget)
        : name(name)
        , vertexGlsl(rsslVertexGlsl)
        , fragmentGlsl(rsslFragmentGlsl)
        , inputTargetNames(inputTargets)
        , outputTargetName(outputTarget)
    {
    }

    const std::string& FullscreenStage::GetName() const
    {
        return name;
    }

    const std::vector<std::string>& FullscreenStage::GetInputTargetNames() const
    {
        return inputTargetNames;
    }

    const std::string& FullscreenStage::GetOutputTargetName() const
    {
        return outputTargetName;
    }

    void FullscreenStage::Initialize(
        IRenderingPlatformBackend* backend,
        const std::unordered_map<std::string, IRenderTarget*>& targets)
    {
        IRenderTarget* target = nullptr;
        auto it = targets.find(outputTargetName);

        if (it != targets.end())
            target = it->second;

        shader = backend->CompileShaderForTarget(vertexGlsl, fragmentGlsl, target, VertexLayout{});

        if (!shader || !shader->IsValid())
        {
            spdlog::error("FullscreenStage '{}': failed to compile shader", name);
            enabled = false;
            return;
        }

        spdlog::info("FullscreenStage '{}': initialized, inputs={}, output='{}'",
            name, inputTargetNames.size(), outputTargetName);
    }

    void FullscreenStage::Execute(StageExecutionContext& context)
    {
        if (!enabled || !shader)
            return;

        auto* target = context.GetTarget(outputTargetName);

        if (!target)
        {
            spdlog::error("FullscreenStage '{}': output target '{}' not found", name, outputTargetName);
            return;
        }

        auto* backend = context.GetBackend();

        std::vector<IRenderTarget*> inputTargets;

        for (const auto& inputName : inputTargetNames)
        {
            auto* inputTarget = context.GetTarget(inputName);

            if (inputTarget)
                inputTargets.push_back(inputTarget);
        }

        backend->BindInputTextures(inputTargets, shader.get(), context.GetFrameIndex());

        backend->BeginRenderTarget(target, true);

        backend->SubmitDrawCommand(shader.get(), nullptr, context.GetFrameIndex(), nullptr);
        backend->ExecuteDrawCommands();

        backend->EndRenderTarget(target);
    }

    bool FullscreenStage::IsEnabled() const
    {
        return enabled;
    }

    void FullscreenStage::SetEnabled(bool enabled)
    {
        this->enabled = enabled;
    }
}
