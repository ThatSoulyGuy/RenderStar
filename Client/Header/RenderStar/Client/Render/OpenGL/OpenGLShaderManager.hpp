#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLShaderProgram;

    class OpenGLShaderManager
    {
    public:

        OpenGLShaderManager();
        ~OpenGLShaderManager();

        OpenGLShaderProgram* GetOrCreateProgram(
            const std::string& name,
            const std::string& vertexSource,
            const std::string& fragmentSource);

        OpenGLShaderProgram* GetProgram(const std::string& name);

        bool HasProgram(const std::string& name) const;

        void DestroyProgram(const std::string& name);

        void DestroyAll();

    private:

        std::shared_ptr<spdlog::logger> logger;
        std::unordered_map<std::string, std::unique_ptr<OpenGLShaderProgram>> programs;
    };
}
