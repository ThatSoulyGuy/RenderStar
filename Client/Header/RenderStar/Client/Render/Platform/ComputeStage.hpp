#pragma once

#include "RenderStar/Client/Render/Platform/IRenderPlatformStage.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace RenderStar::Client::Render::Platform
{
    class ComputeStage : public IRenderPlatformStage
    {
    public:

        ComputeStage(
            const std::string& name,
            const std::string& rsslComputeGlsl,
            const std::vector<std::string>& inputTargets,
            const std::string& outputTarget,
            uint32_t groupSizeX,
            uint32_t groupSizeY,
            uint32_t groupSizeZ);

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
        std::string computeGlsl;
        std::vector<std::string> inputTargetNames;
        std::string outputTargetName;
        uint32_t groupSizeX;
        uint32_t groupSizeY;
        uint32_t groupSizeZ;
        bool enabled = true;
        std::unique_ptr<IShaderProgram> shader;
    };
}
