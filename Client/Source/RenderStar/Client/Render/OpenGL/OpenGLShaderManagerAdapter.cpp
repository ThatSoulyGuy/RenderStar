#include "RenderStar/Client/Render/OpenGL/OpenGLShaderManagerAdapter.hpp"

#include "RenderStar/Client/Render/OpenGL/OpenGLShaderProgram.hpp"
#include "RenderStar/Common/Asset/ITextAsset.hpp"
#include "RenderStar/Common/Asset/IBinaryAsset.hpp"

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLShaderManagerAdapter::OpenGLShaderManagerAdapter()
        : logger(spdlog::default_logger())
    {
    }

    OpenGLShaderManagerAdapter::~OpenGLShaderManagerAdapter()
    {
        internalManager.DestroyAll();
    }

    std::unique_ptr<IShaderProgram> OpenGLShaderManagerAdapter::CreateFromSource(const ShaderSource& source)
    {
        auto program = std::make_unique<OpenGLShaderProgram>();

        if (!program->CompileFromSource(source.vertexSource, source.fragmentSource))
        {
            logger->error("Failed to compile shader from source");
            return nullptr;
        }

        return program;
    }

    std::unique_ptr<IShaderProgram> OpenGLShaderManagerAdapter::CreateFromBinary(const ShaderBinary& binary)
    {
        logger->warn("Binary shader loading not supported in OpenGL adapter");
        return nullptr;
    }

    std::unique_ptr<IShaderProgram> OpenGLShaderManagerAdapter::CreateFromTextAssets(const Common::Asset::ITextAsset& vertexAsset, const Common::Asset::ITextAsset& fragmentAsset)
    {
        ShaderSource source;
        source.vertexSource = vertexAsset.GetContent();
        source.fragmentSource = fragmentAsset.GetContent();

        return CreateFromSource(source);
    }

    std::unique_ptr<IShaderProgram> OpenGLShaderManagerAdapter::CreateFromBinaryAssets(const Common::Asset::IBinaryAsset& vertexAsset, const Common::Asset::IBinaryAsset& fragmentAsset)
    {
        logger->warn("Binary shader loading not supported in OpenGL adapter - use text assets");
        return nullptr;
    }

    std::unique_ptr<IShaderProgram> OpenGLShaderManagerAdapter::CreateComputeFromTextAsset(const Common::Asset::ITextAsset& computeAsset)
    {
        logger->warn("Compute shader not yet implemented in OpenGL adapter");
        return nullptr;
    }

    std::unique_ptr<IShaderProgram> OpenGLShaderManagerAdapter::CreateComputeFromBinaryAsset(const Common::Asset::IBinaryAsset& computeAsset)
    {
        logger->warn("Compute shader from binary not supported in OpenGL adapter");
        return nullptr;
    }

    void OpenGLShaderManagerAdapter::DestroyShader(IShaderProgram* shader)
    {
    }
}
