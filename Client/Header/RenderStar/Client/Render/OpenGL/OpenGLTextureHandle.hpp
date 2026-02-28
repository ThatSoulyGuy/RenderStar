#pragma once

#include "RenderStar/Client/Render/Resource/ITextureHandle.hpp"
#include <cstdint>

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLTextureHandle : public ITextureHandle
    {
    public:

        OpenGLTextureHandle(uint32_t textureId, uint32_t width, uint32_t height);
        ~OpenGLTextureHandle() override;

        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;
        bool IsValid() const override;

        uint32_t GetTextureId() const;

    private:

        uint32_t textureId;
        uint32_t width;
        uint32_t height;
    };
}
