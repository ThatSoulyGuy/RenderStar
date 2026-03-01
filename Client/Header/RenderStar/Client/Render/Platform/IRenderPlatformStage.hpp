#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace RenderStar::Client::Render::Platform
{
    class IRenderTarget;
    class IRenderingPlatformBackend;
    class StageExecutionContext;

    class IRenderPlatformStage
    {
    public:

        virtual ~IRenderPlatformStage() = default;

        virtual const std::string& GetName() const = 0;

        virtual const std::vector<std::string>& GetInputTargetNames() const = 0;

        virtual const std::string& GetOutputTargetName() const = 0;

        virtual void Initialize(
            IRenderingPlatformBackend* backend,
            const std::unordered_map<std::string, IRenderTarget*>& targets) = 0;

        virtual void Execute(StageExecutionContext& context) = 0;

        virtual bool IsEnabled() const = 0;

        virtual void SetEnabled(bool enabled) = 0;
    };
}
