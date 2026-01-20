#pragma once

#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include <cstdint>

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLBufferHandle : public IBufferHandle
    {
    public:

        OpenGLBufferHandle(uint32_t bufferId, BufferType type, BufferUsage usage, size_t size);
        ~OpenGLBufferHandle() override;

        void Bind();
        void Unbind();

        void SetData(const void* data, size_t size) override;
        void SetSubData(const void* data, size_t size, size_t offset) override;

        size_t GetSize() const override;
        BufferType GetType() const override;
        BufferUsage GetUsage() const override;

        bool IsValid() const override;

        uint32_t GetBufferId() const;
        bool IsDestroyed() const;

    private:

        uint32_t bufferId;
        BufferType type;
        BufferUsage usage;
        size_t size;
        bool destroyed;
    };
}
