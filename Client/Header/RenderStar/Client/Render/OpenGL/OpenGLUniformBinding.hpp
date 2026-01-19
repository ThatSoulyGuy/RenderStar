#pragma once

#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/UniformLayout.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLBufferHandle;

    class OpenGLUniformBinding : public IUniformBindingHandle
    {
    public:

        OpenGLUniformBinding(const UniformLayout& layout, int32_t frameCount);
        ~OpenGLUniformBinding() override;

        void Bind(int32_t frameIndex) override;
        void UpdateBuffer(int32_t binding, IBufferHandle* buffer, size_t size) override;
        uint64_t GetNativeHandle(int32_t frameIndex) const override;
        void Destroy() override;
        bool IsDestroyed() const override;

        void UpdateBufferForFrame(int32_t binding, int32_t frameIndex, OpenGLBufferHandle* buffer);

        const UniformLayout& GetLayout() const;

    private:

        std::shared_ptr<spdlog::logger> logger;
        UniformLayout layout;
        int32_t frameCount;
        std::unordered_map<int32_t, std::vector<OpenGLBufferHandle*>> bindingBuffers;
        bool destroyed;
    };
}
