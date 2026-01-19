#pragma once

#include "RenderStar/Common/Utility/ResourcePath.hpp"
#include "RenderStar/Client/Render/Backend/RenderBackend.hpp"
#include "RenderStar/Client/Render/Shader/GlslTransformer.hpp"
#include <string>
#include <filesystem>
#include <stdexcept>

namespace RenderStar::Client::Render::Shader
{
    using RenderStar::Common::Utility::ResourcePath;

    class ShaderResolutionException : public std::runtime_error
    {
    public:
        explicit ShaderResolutionException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    class ShaderResolver
    {
    public:

        static void SetResourceBasePath(const std::filesystem::path& path);

        static std::string Resolve(
            const ResourcePath& path,
            RenderBackend backend,
            ShaderType shaderType);

        static std::string ResolveVertex(const ResourcePath& path, RenderBackend backend);

        static std::string ResolveFragment(const ResourcePath& path, RenderBackend backend);

        static bool HasBackendOverride(const ResourcePath& path, RenderBackend backend);

        static ResourcePath GetBackendOverridePath(const ResourcePath& path, RenderBackend backend);

    private:

        static std::string TryLoadResource(const ResourcePath& path);

        static bool ResourceExists(const ResourcePath& path);

        static std::string GetBackendDirectoryName(RenderBackend backend);

        static std::filesystem::path resourceBasePath;
    };
}
