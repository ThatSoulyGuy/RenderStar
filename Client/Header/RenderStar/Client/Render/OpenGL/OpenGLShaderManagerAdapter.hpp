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

        std::unique_ptr<IShaderProgram> LoadFromFile(const Common::Asset::AssetModule&, const Common::Asset::AssetLocation&, const Common::Asset::AssetLocation&) override;

        std::unique_ptr<IShaderProgram> LoadComputeFromFile(const std::string& computePath) override;

        void DestroyShader(IShaderProgram* shader) override;

    private:

        std::shared_ptr<spdlog::logger> logger;
        OpenGLShaderManager internalManager;
    };
}
