#pragma once

#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include <memory>
#include <string>
#include <vector>

namespace RenderStar::Common::Asset
{
    class ITextAsset;
    class IBinaryAsset;
}

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

        virtual std::unique_ptr<IShaderProgram> CreateFromTextAssets(const Common::Asset::ITextAsset& vertexAsset, const Common::Asset::ITextAsset& fragmentAsset) = 0;
        virtual std::unique_ptr<IShaderProgram> CreateFromBinaryAssets(const Common::Asset::IBinaryAsset& vertexAsset, const Common::Asset::IBinaryAsset& fragmentAsset) = 0;

        virtual std::unique_ptr<IShaderProgram> CreateComputeFromTextAsset(const Common::Asset::ITextAsset& computeAsset) = 0;
        virtual std::unique_ptr<IShaderProgram> CreateComputeFromBinaryAsset(const Common::Asset::IBinaryAsset& computeAsset) = 0;

        virtual void DestroyShader(IShaderProgram* shader) = 0;
    };
}
