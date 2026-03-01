#include "RenderStar/Client/Render/Platform/StageExecutionContext.hpp"

namespace RenderStar::Client::Render::Platform
{
    StageExecutionContext::StageExecutionContext(
        IRenderingPlatformBackend* backend,
        const std::unordered_map<std::string, IRenderTarget*>& targets,
        int32_t frameIndex)
        : backend(backend)
        , targets(targets)
        , frameIndex(frameIndex)
    {
    }

    IRenderTarget* StageExecutionContext::GetTarget(const std::string& name) const
    {
        auto it = targets.find(name);

        if (it != targets.end())
            return it->second;

        return nullptr;
    }

    IRenderingPlatformBackend* StageExecutionContext::GetBackend() const
    {
        return backend;
    }

    int32_t StageExecutionContext::GetFrameIndex() const
    {
        return frameIndex;
    }

    const std::vector<DrawCommand>& StageExecutionContext::GetDrawCommands() const
    {
        return drawCommands;
    }

    void StageExecutionContext::SubmitDrawCommands(const std::vector<DrawCommand>& commands)
    {
        drawCommands.insert(drawCommands.end(), commands.begin(), commands.end());
    }
}
