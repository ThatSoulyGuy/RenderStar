#include "RenderStar/Client/Render/Platform/GeometryStage.hpp"
#include "RenderStar/Client/Render/Platform/StageExecutionContext.hpp"
#include "RenderStar/Client/Render/Platform/IRenderingPlatformBackend.hpp"
#include "RenderStar/Client/Render/Platform/IRenderTarget.hpp"
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Platform
{
    GeometryStage::GeometryStage(
        const std::string& name,
        const std::string& rsslVertexGlsl,
        const std::string& rsslFragmentGlsl,
        const std::string& outputTarget,
        bool clearOnBegin,
        const VertexLayout& vertexLayout)
        : name(name)
        , vertexGlsl(rsslVertexGlsl)
        , fragmentGlsl(rsslFragmentGlsl)
        , outputTargetName(outputTarget)
        , clearOnBegin(clearOnBegin)
        , vertexLayout(vertexLayout)
    {
    }

    const std::string& GeometryStage::GetName() const
    {
        return name;
    }

    const std::vector<std::string>& GeometryStage::GetInputTargetNames() const
    {
        return inputTargetNames;
    }

    const std::string& GeometryStage::GetOutputTargetName() const
    {
        return outputTargetName;
    }

    void GeometryStage::Initialize(
        IRenderingPlatformBackend* backend,
        const std::unordered_map<std::string, IRenderTarget*>& targets)
    {
        IRenderTarget* target = nullptr;
        auto it = targets.find(outputTargetName);

        if (it != targets.end())
            target = it->second;

        shader = backend->CompileShaderForTarget(vertexGlsl, fragmentGlsl, target, vertexLayout);

        if (!shader || !shader->IsValid())
        {
            spdlog::error("GeometryStage '{}': failed to compile shader", name);
            enabled = false;
            return;
        }

        spdlog::info("GeometryStage '{}': initialized, output='{}'", name, outputTargetName);
    }

    void GeometryStage::Execute(StageExecutionContext& context)
    {
        if (!enabled || !shader)
            return;

        auto* target = context.GetTarget(outputTargetName);

        if (!target)
        {
            spdlog::error("GeometryStage '{}': output target '{}' not found", name, outputTargetName);
            return;
        }

        auto* backend = context.GetBackend();

        backend->BeginRenderTarget(target, clearOnBegin);
        backend->ExecuteDrawCommands();
        backend->EndRenderTarget(target);
    }

    bool GeometryStage::IsEnabled() const
    {
        return enabled;
    }

    void GeometryStage::SetEnabled(bool enabled)
    {
        this->enabled = enabled;
    }

    IShaderProgram* GeometryStage::GetShader() const
    {
        return shader.get();
    }
}
