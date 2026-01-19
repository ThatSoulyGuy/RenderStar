#include "RenderStar/Client/Render/OpenGL/OpenGLShaderProgram.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLShaderProgram::OpenGLShaderProgram()
        : logger(spdlog::default_logger())
        , programHandle(INVALID_PROGRAM)
    {
    }

    OpenGLShaderProgram::~OpenGLShaderProgram()
    {
        if (programHandle != INVALID_PROGRAM)
        {
            glDeleteProgram(programHandle);
            programHandle = INVALID_PROGRAM;
        }
    }

    bool OpenGLShaderProgram::CompileFromSource(const std::string& vertexSource, const std::string& fragmentSource)
    {
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* vertexSourcePtr = vertexSource.c_str();
        glShaderSource(vertexShader, 1, &vertexSourcePtr, nullptr);
        glCompileShader(vertexShader);

        if (!CheckShaderCompilation(vertexShader, "vertex"))
        {
            glDeleteShader(vertexShader);
            return false;
        }

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fragmentSourcePtr = fragmentSource.c_str();
        glShaderSource(fragmentShader, 1, &fragmentSourcePtr, nullptr);
        glCompileShader(fragmentShader);

        if (!CheckShaderCompilation(fragmentShader, "fragment"))
        {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return false;
        }

        programHandle = glCreateProgram();
        glAttachShader(programHandle, vertexShader);
        glAttachShader(programHandle, fragmentShader);
        glLinkProgram(programHandle);

        if (!CheckProgramLinking())
        {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            glDeleteProgram(programHandle);
            programHandle = INVALID_PROGRAM;
            return false;
        }

        glDetachShader(programHandle, vertexShader);
        glDetachShader(programHandle, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        logger->info("Shader program {} compiled successfully", programHandle);
        return true;
    }

    void OpenGLShaderProgram::Bind()
    {
        if (programHandle != INVALID_PROGRAM)
            glUseProgram(programHandle);
    }

    void OpenGLShaderProgram::Unbind()
    {
        glUseProgram(0);
    }

    ShaderProgramHandle OpenGLShaderProgram::GetHandle() const
    {
        return programHandle;
    }

    bool OpenGLShaderProgram::IsValid() const
    {
        return programHandle != INVALID_PROGRAM;
    }

    void OpenGLShaderProgram::SetUniformBlockBinding(const std::string& blockName, uint32_t bindingPoint)
    {
        if (programHandle == INVALID_PROGRAM)
            return;

        GLuint blockIndex = glGetUniformBlockIndex(programHandle, blockName.c_str());

        if (blockIndex != GL_INVALID_INDEX)
        {
            glUniformBlockBinding(programHandle, blockIndex, bindingPoint);
            logger->debug("Bound uniform block '{}' to binding point {}", blockName, bindingPoint);
        }
        else
        {
            logger->warn("Uniform block '{}' not found in shader program {}", blockName, programHandle);
        }
    }

    void OpenGLShaderProgram::SetUniform(const std::string& name, int32_t value)
    {
        GLint location = GetUniformLocation(name);
        if (location >= 0)
            glUniform1i(location, value);
    }

    void OpenGLShaderProgram::SetUniform(const std::string& name, float value)
    {
        GLint location = GetUniformLocation(name);
        if (location >= 0)
            glUniform1f(location, value);
    }

    void OpenGLShaderProgram::SetUniform(const std::string& name, const glm::vec2& value)
    {
        GLint location = GetUniformLocation(name);
        if (location >= 0)
            glUniform2f(location, value.x, value.y);
    }

    void OpenGLShaderProgram::SetUniform(const std::string& name, const glm::vec3& value)
    {
        GLint location = GetUniformLocation(name);
        if (location >= 0)
            glUniform3f(location, value.x, value.y, value.z);
    }

    void OpenGLShaderProgram::SetUniform(const std::string& name, const glm::vec4& value)
    {
        GLint location = GetUniformLocation(name);
        if (location >= 0)
            glUniform4f(location, value.x, value.y, value.z, value.w);
    }

    void OpenGLShaderProgram::SetUniform(const std::string& name, const glm::mat3& value)
    {
        GLint location = GetUniformLocation(name);
        if (location >= 0)
            glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]);
    }

    void OpenGLShaderProgram::SetUniform(const std::string& name, const glm::mat4& value)
    {
        GLint location = GetUniformLocation(name);
        if (location >= 0)
            glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
    }

    int32_t OpenGLShaderProgram::GetUniformLocation(const std::string& name) const
    {
        if (programHandle == INVALID_PROGRAM)
            return -1;
        return glGetUniformLocation(programHandle, name.c_str());
    }

    bool OpenGLShaderProgram::CheckShaderCompilation(uint32_t shader, const std::string& type)
    {
        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (success == GL_FALSE)
        {
            GLint logLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

            std::vector<char> infoLog(logLength);
            glGetShaderInfoLog(shader, logLength, nullptr, infoLog.data());

            logger->error("Failed to compile {} shader:\n{}", type, infoLog.data());
            return false;
        }

        return true;
    }

    bool OpenGLShaderProgram::CheckProgramLinking()
    {
        GLint success = 0;
        glGetProgramiv(programHandle, GL_LINK_STATUS, &success);

        if (success == GL_FALSE)
        {
            GLint logLength = 0;
            glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLength);

            std::vector<char> infoLog(logLength);
            glGetProgramInfoLog(programHandle, logLength, nullptr, infoLog.data());

            logger->error("Failed to link shader program:\n{}", infoLog.data());
            return false;
        }

        return true;
    }
}
