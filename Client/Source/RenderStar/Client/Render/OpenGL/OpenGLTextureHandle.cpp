#include "RenderStar/Client/Render/OpenGL/OpenGLTextureHandle.hpp"
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLTextureHandle::OpenGLTextureHandle(uint32_t textureId, uint32_t width, uint32_t height)
        : textureId(textureId)
        , width(width)
        , height(height)
    {
    }

    OpenGLTextureHandle::~OpenGLTextureHandle()
    {
        if (textureId != 0)
        {
            glDeleteTextures(1, &textureId);
            textureId = 0;
        }
    }

    uint32_t OpenGLTextureHandle::GetWidth() const
    {
        return width;
    }

    uint32_t OpenGLTextureHandle::GetHeight() const
    {
        return height;
    }

    bool OpenGLTextureHandle::IsValid() const
    {
        return textureId != 0;
    }

    uint32_t OpenGLTextureHandle::GetTextureId() const
    {
        return textureId;
    }
}
