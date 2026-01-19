#pragma once

#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include <memory>
#include <string>
#include <vector>

namespace RenderStar::Client::Render
{
    struct ShaderSource
    {
        std::string vertexSource;
        std::string fragmentSource;
        std::string computeSource;
    };

    struct ShaderBinary
    {
        std::vector<uint32_t> vertexSpirv;
        std::vector<uint32_t> fragmentSpirv;
        std::vector<uint32_t> computeSpirv;
    };

    class IShaderManager
    {
    public:

        virtual ~IShaderManager() = default;

        virtual std::unique_ptr<IShaderProgram> CreateFromSource(const ShaderSource& source) = 0;
        virtual std::unique_ptr<IShaderProgram> CreateFromBinary(const ShaderBinary& binary) = 0;

        virtual std::unique_ptr<IShaderProgram> LoadFromFile(
            const std::string& vertexPath,
            const std::string& fragmentPath) = 0;

        virtual std::unique_ptr<IShaderProgram> LoadComputeFromFile(const std::string& computePath) = 0;

        virtual void DestroyShader(IShaderProgram* shader) = 0;
    };
}
