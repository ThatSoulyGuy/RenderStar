#pragma once

#include "RenderStar/Client/Render/Resource/IUniformManager.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLBufferManagerAdapter.hpp"
#include <unordered_map>
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLUniformManagerAdapter : public IUniformManager
    {
    public:

        OpenGLUniformManagerAdapter();
        ~OpenGLUniformManagerAdapter() override;

        void CreateUniformBuffer(
            const std::string& name,
            const UniformBinding& binding) override;

        void UpdateUniformBuffer(
            const std::string& name,
            const void* data,
            size_t size,
            size_t offset = 0) override;

        void BindUniformBuffer(const std::string& name, uint32_t bindingPoint) override;

        IBufferHandle* GetUniformBuffer(const std::string& name) override;

        void DestroyUniformBuffer(const std::string& name) override;

        std::unique_ptr<IUniformBindingHandle> CreateBindingForShader(IShaderProgram* shader) override;

    private:

        std::shared_ptr<spdlog::logger> logger;
        std::unique_ptr<OpenGLBufferManagerAdapter> bufferManager;
        std::unordered_map<std::string, std::unique_ptr<IBufferHandle>> uniformBuffers;
        std::unordered_map<std::string, uint32_t> bufferBindings;
    };
}
