#pragma once

#include "RenderStar/Client/Render/Resource/UniformLayout.hpp"
#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include <memory>
#include <string>

namespace RenderStar::Client::Render
{
    class IShaderProgram;

    class IUniformManager
    {
    public:

        virtual ~IUniformManager() = default;

        virtual void CreateUniformBuffer(
            const std::string& name,
            const UniformBinding& binding) = 0;

        virtual void UpdateUniformBuffer(
            const std::string& name,
            const void* data,
            size_t size,
            size_t offset = 0) = 0;

        virtual void BindUniformBuffer(const std::string& name, uint32_t bindingPoint) = 0;

        virtual IBufferHandle* GetUniformBuffer(const std::string& name) = 0;

        virtual void DestroyUniformBuffer(const std::string& name) = 0;

        virtual std::unique_ptr<IUniformBindingHandle> CreateBindingForShader(IShaderProgram* shader) = 0;
    };
}
