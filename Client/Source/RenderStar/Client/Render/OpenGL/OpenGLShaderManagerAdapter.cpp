#include "RenderStar/Client/Render/OpenGL/OpenGLShaderManagerAdapter.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLShaderProgram.hpp"
#include <fstream>
#include <sstream>

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

    std::unique_ptr<IShaderProgram> OpenGLShaderManagerAdapter::LoadFromFile(
        const std::string& vertexPath,
        const std::string& fragmentPath)
    {
        auto readFile = [](const std::string& path) -> std::string {
            std::ifstream file(path);
            if (!file.is_open())
                return "";

            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        };

        std::string vertexSource = readFile(vertexPath);
        std::string fragmentSource = readFile(fragmentPath);

        if (vertexSource.empty())
        {
            logger->error("Failed to read vertex shader from: {}", vertexPath);
            return nullptr;
        }

        if (fragmentSource.empty())
        {
            logger->error("Failed to read fragment shader from: {}", fragmentPath);
            return nullptr;
        }

        ShaderSource source;
        source.vertexSource = std::move(vertexSource);
        source.fragmentSource = std::move(fragmentSource);

        return CreateFromSource(source);
    }

    std::unique_ptr<IShaderProgram> OpenGLShaderManagerAdapter::LoadComputeFromFile(const std::string& computePath)
    {
        logger->warn("Compute shader not yet implemented in OpenGL adapter");
        return nullptr;
    }

    void OpenGLShaderManagerAdapter::DestroyShader(IShaderProgram* shader)
    {
    }
}
