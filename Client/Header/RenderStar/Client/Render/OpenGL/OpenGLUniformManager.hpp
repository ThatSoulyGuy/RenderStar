#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    using UniformBufferHandle = uint32_t;

    struct UniformBlockInfo
    {
        std::string name;
        uint32_t bindingPoint;
        UniformBufferHandle bufferHandle;
        size_t bufferSize;
    };

    class OpenGLUniformManager
    {
    public:

        OpenGLUniformManager();

        ~OpenGLUniformManager();

        void RegisterUniformBlock(const std::string& blockName, uint32_t bindingPoint, size_t bufferSize);

        void UpdateUniformBlock(const std::string& blockName, const void* data, size_t size, size_t offset);

        void BindUniformBlock(const std::string& blockName);

        void UnbindUniformBlock(const std::string& blockName);

        bool HasUniformBlock(const std::string& blockName) const;

        uint32_t GetBindingPoint(const std::string& blockName) const;

    private:

        std::shared_ptr<spdlog::logger> logger;
        std::unordered_map<std::string, UniformBlockInfo> uniformBlocks;
    };
}
