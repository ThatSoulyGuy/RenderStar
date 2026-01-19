#include "RenderStar/Client/Render/OpenGL/OpenGLShaderManager.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLShaderProgram.hpp"

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLShaderManager::OpenGLShaderManager()
        : logger(spdlog::default_logger())
    {
    }

    OpenGLShaderManager::~OpenGLShaderManager()
    {
        DestroyAll();
    }

    OpenGLShaderProgram* OpenGLShaderManager::GetOrCreateProgram(
        const std::string& name,
        const std::string& vertexSource,
        const std::string& fragmentSource)
    {
        auto it = programs.find(name);
        if (it != programs.end())
            return it->second.get();

        auto program = std::make_unique<OpenGLShaderProgram>();

        if (!program->CompileFromSource(vertexSource, fragmentSource))
        {
            logger->error("Failed to create shader program '{}'", name);
            return nullptr;
        }

        auto* rawPtr = program.get();
        programs[name] = std::move(program);

        logger->info("Created OpenGL shader program '{}'", name);
        return rawPtr;
    }

    OpenGLShaderProgram* OpenGLShaderManager::GetProgram(const std::string& name)
    {
        auto it = programs.find(name);
        if (it != programs.end())
            return it->second.get();
        return nullptr;
    }

    bool OpenGLShaderManager::HasProgram(const std::string& name) const
    {
        return programs.find(name) != programs.end();
    }

    void OpenGLShaderManager::DestroyProgram(const std::string& name)
    {
        auto it = programs.find(name);
        if (it != programs.end())
        {
            programs.erase(it);
            logger->debug("Destroyed shader program '{}'", name);
        }
    }

    void OpenGLShaderManager::DestroyAll()
    {
        programs.clear();
        logger->info("Destroyed all OpenGL shader programs");
    }
}
