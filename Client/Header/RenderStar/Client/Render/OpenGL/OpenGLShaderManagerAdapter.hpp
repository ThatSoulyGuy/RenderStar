#pragma once

#include "RenderStar/Client/Render/Resource/IShaderManager.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLShaderManager.hpp"
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLShaderManagerAdapter : public IShaderManager
    {
    public:

        OpenGLShaderManagerAdapter();
        ~OpenGLShaderManagerAdapter() override;

        std::unique_ptr<IShaderProgram> CreateFromSource(const ShaderSource& source) override;
        std::unique_ptr<IShaderProgram> CreateFromBinary(const ShaderBinary& binary) override;

        std::unique_ptr<IShaderProgram> CreateFromTextAssets(const Common::Asset::ITextAsset& vertexAsset, const Common::Asset::ITextAsset& fragmentAsset) override;
        std::unique_ptr<IShaderProgram> CreateFromBinaryAssets(const Common::Asset::IBinaryAsset& vertexAsset, const Common::Asset::IBinaryAsset& fragmentAsset) override;

        std::unique_ptr<IShaderProgram> CreateComputeFromTextAsset(const Common::Asset::ITextAsset& computeAsset) override;
        std::unique_ptr<IShaderProgram> CreateComputeFromBinaryAsset(const Common::Asset::IBinaryAsset& computeAsset) override;

        void DestroyShader(IShaderProgram* shader) override;

    private:

        std::shared_ptr<spdlog::logger> logger;
        OpenGLShaderManager internalManager;
    };
}
