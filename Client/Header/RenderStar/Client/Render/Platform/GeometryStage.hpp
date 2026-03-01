#pragma once

#include "RenderStar/Client/Render/Platform/IRenderPlatformStage.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include <memory>
#include <string>
#include <vector>

namespace RenderStar::Client::Render::Platform
{
    class GeometryStage : public IRenderPlatformStage
    {
    public:

        GeometryStage(
            const std::string& name,
            const std::string& rsslVertexGlsl,
            const std::string& rsslFragmentGlsl,
            const std::string& outputTarget,
            bool clearOnBegin,
            const VertexLayout& vertexLayout = VertexLayout{});

        const std::string& GetName() const override;
        const std::vector<std::string>& GetInputTargetNames() const override;
        const std::string& GetOutputTargetName() const override;
        void Initialize(
            IRenderingPlatformBackend* backend,
            const std::unordered_map<std::string, IRenderTarget*>& targets) override;
        void Execute(StageExecutionContext& context) override;
        bool IsEnabled() const override;
        void SetEnabled(bool enabled) override;

        IShaderProgram* GetShader() const;

    private:

        std::string name;
        std::string vertexGlsl;
        std::string fragmentGlsl;
        std::string outputTargetName;
        std::vector<std::string> inputTargetNames;
        bool clearOnBegin;
        bool enabled = true;
        VertexLayout vertexLayout;
        std::unique_ptr<IShaderProgram> shader;
    };
}
