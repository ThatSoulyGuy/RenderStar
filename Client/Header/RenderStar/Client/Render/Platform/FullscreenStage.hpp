#pragma once

#include "RenderStar/Client/Render/Platform/IRenderPlatformStage.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include <memory>
#include <string>
#include <vector>

namespace RenderStar::Client::Render::Platform
{
    class FullscreenStage : public IRenderPlatformStage
    {
    public:

        FullscreenStage(
            const std::string& name,
            const std::string& rsslVertexGlsl,
            const std::string& rsslFragmentGlsl,
            const std::vector<std::string>& inputTargets,
            const std::string& outputTarget);

        const std::string& GetName() const override;
        const std::vector<std::string>& GetInputTargetNames() const override;
        const std::string& GetOutputTargetName() const override;
        void Initialize(
            IRenderingPlatformBackend* backend,
            const std::unordered_map<std::string, IRenderTarget*>& targets) override;
        void Execute(StageExecutionContext& context) override;
        bool IsEnabled() const override;
        void SetEnabled(bool enabled) override;

    private:

        std::string name;
        std::string vertexGlsl;
        std::string fragmentGlsl;
        std::vector<std::string> inputTargetNames;
        std::string outputTargetName;
        bool enabled = true;
        std::unique_ptr<IShaderProgram> shader;
    };
}
