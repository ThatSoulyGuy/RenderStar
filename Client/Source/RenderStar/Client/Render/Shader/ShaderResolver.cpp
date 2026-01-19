#include "RenderStar/Client/Render/Shader/ShaderResolver.hpp"
#include "RenderStar/Client/Render/Shader/GlslTransformer.hpp"
#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::Shader
{
    std::filesystem::path ShaderResolver::resourceBasePath = std::filesystem::current_path();

    void ShaderResolver::SetResourceBasePath(const std::filesystem::path& path)
    {
        resourceBasePath = path;
    }

    std::string ShaderResolver::Resolve(
        const ResourcePath& path,
        RenderBackend backend,
        ShaderType shaderType)
    {
        ResourcePath overridePath = GetBackendOverridePath(path, backend);
        std::string overrideSource = TryLoadResource(overridePath);

        if (!overrideSource.empty())
        {
            spdlog::debug("Using backend-specific shader: {}", overridePath.FullPath());
            return overrideSource;
        }

        std::string primarySource = TryLoadResource(path);

        if (primarySource.empty())
            throw ShaderResolutionException("Failed to load shader: " + path.FullPath());

        if (backend == RenderBackend::OPENGL && GlslTransformer::CanTransform(primarySource))
        {
            spdlog::debug("Transforming shader {} from GLSL 450 to GLSL 410 for OpenGL", path.localPath);
            return GlslTransformer::Transform450To410(primarySource, shaderType);
        }

        return primarySource;
    }

    std::string ShaderResolver::ResolveVertex(const ResourcePath& path, RenderBackend backend)
    {
        return Resolve(path, backend, ShaderType::VERTEX);
    }

    std::string ShaderResolver::ResolveFragment(const ResourcePath& path, RenderBackend backend)
    {
        return Resolve(path, backend, ShaderType::FRAGMENT);
    }

    bool ShaderResolver::HasBackendOverride(const ResourcePath& path, RenderBackend backend)
    {
        ResourcePath overridePath = GetBackendOverridePath(path, backend);
        return ResourceExists(overridePath);
    }

    ResourcePath ShaderResolver::GetBackendOverridePath(const ResourcePath& path, RenderBackend backend)
    {
        std::string localPath = path.localPath;
        std::string backendDir = GetBackendDirectoryName(backend);

        size_t lastSlash = localPath.rfind('/');

        std::string newPath;

        if (lastSlash != std::string::npos)
        {
            std::string directory = localPath.substr(0, lastSlash);
            std::string filename = localPath.substr(lastSlash + 1);
            newPath = directory + "/" + backendDir + "/" + filename;
        }
        else
        {
            newPath = backendDir + "/" + localPath;
        }

        return ResourcePath(path.domain, newPath);
    }

    std::string ShaderResolver::TryLoadResource(const ResourcePath& path)
    {
        std::filesystem::path filePath = path.ToFilesystemPath(resourceBasePath);

        if (!std::filesystem::exists(filePath))
            return "";

        std::ifstream file(filePath, std::ios::in);

        if (!file.is_open())
        {
            spdlog::warn("Failed to open resource {}: {}", path.FullPath(), filePath.string());
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        return buffer.str();
    }

    bool ShaderResolver::ResourceExists(const ResourcePath& path)
    {
        std::filesystem::path filePath = path.ToFilesystemPath(resourceBasePath);
        return std::filesystem::exists(filePath);
    }

    std::string ShaderResolver::GetBackendDirectoryName(RenderBackend backend)
    {
        switch (backend)
        {
            case RenderBackend::OPENGL: return "opengl";
            case RenderBackend::VULKAN: return "vulkan";
        }
        return "opengl";
    }
}
