#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace RenderStar::Client::Render
{
    class IShaderProgram;
    class IUniformBindingHandle;
    class IMesh;
}

namespace RenderStar::Client::Render::Platform
{
    class IRenderTarget;
    class IRenderingPlatformBackend;

    struct DrawCommand
    {
        IShaderProgram* shader = nullptr;
        IUniformBindingHandle* uniformBinding = nullptr;
        IMesh* mesh = nullptr;
    };

    class StageExecutionContext
    {
    public:

        StageExecutionContext(
            IRenderingPlatformBackend* backend,
            const std::unordered_map<std::string, IRenderTarget*>& targets,
            int32_t frameIndex);

        IRenderTarget* GetTarget(const std::string& name) const;

        IRenderingPlatformBackend* GetBackend() const;

        int32_t GetFrameIndex() const;

        const std::vector<DrawCommand>& GetDrawCommands() const;

        void SubmitDrawCommands(const std::vector<DrawCommand>& commands);

    private:

        IRenderingPlatformBackend* backend;
        const std::unordered_map<std::string, IRenderTarget*>& targets;
        int32_t frameIndex;
        std::vector<DrawCommand> drawCommands;
    };
}
