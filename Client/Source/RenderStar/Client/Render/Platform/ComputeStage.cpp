#include "RenderStar/Client/Render/Platform/ComputeStage.hpp"
#include "RenderStar/Client/Render/Platform/StageExecutionContext.hpp"
#include "RenderStar/Client/Render/Platform/IRenderingPlatformBackend.hpp"
#include "RenderStar/Client/Render/Platform/IRenderTarget.hpp"
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Platform
{
    ComputeStage::ComputeStage(
        const std::string& name,
        const std::string& rsslComputeGlsl,
        const std::vector<std::string>& inputTargets,
        const std::string& outputTarget,
        uint32_t groupSizeX,
        uint32_t groupSizeY,
        uint32_t groupSizeZ)
        : name(name)
        , computeGlsl(rsslComputeGlsl)
        , inputTargetNames(inputTargets)
        , outputTargetName(outputTarget)
        , groupSizeX(groupSizeX)
        , groupSizeY(groupSizeY)
        , groupSizeZ(groupSizeZ)
    {
    }

    const std::string& ComputeStage::GetName() const
    {
        return name;
    }

    const std::vector<std::string>& ComputeStage::GetInputTargetNames() const
    {
        return inputTargetNames;
    }

    const std::string& ComputeStage::GetOutputTargetName() const
    {
        return outputTargetName;
    }

    void ComputeStage::Initialize(
        IRenderingPlatformBackend* backend,
        const std::unordered_map<std::string, IRenderTarget*>&)
    {
        shader = backend->CompileComputeShader(computeGlsl);

        if (!shader || !shader->IsValid())
        {
            spdlog::error("ComputeStage '{}': failed to compile compute shader", name);
            enabled = false;
            return;
        }

        spdlog::info("ComputeStage '{}': initialized, group=({},{},{}), output='{}'",
            name, groupSizeX, groupSizeY, groupSizeZ, outputTargetName);
    }

    void ComputeStage::Execute(StageExecutionContext& context)
    {
        if (!enabled || !shader)
            return;

        auto* backend = context.GetBackend();

        backend->BindInputTextures({}, shader.get(), context.GetFrameIndex());

        spdlog::trace("ComputeStage '{}': dispatch not yet implemented for this backend", name);
    }

    bool ComputeStage::IsEnabled() const
    {
        return enabled;
    }

    void ComputeStage::SetEnabled(bool enabled)
    {
        this->enabled = enabled;
    }
}
