#include "RenderStar/Client/Render/OpenGL/OpenGLTextureManager.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLTextureHandle.hpp"
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    namespace
    {
        GLenum ToGLWrapMode(TextureWrapMode mode)
        {
            switch (mode)
            {
                case TextureWrapMode::REPEAT: return GL_REPEAT;
                case TextureWrapMode::CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
            }
            return GL_REPEAT;
        }

        GLenum ToGLFilter(TextureFilterMode mode)
        {
            switch (mode)
            {
                case TextureFilterMode::NEAREST: return GL_NEAREST;
                case TextureFilterMode::LINEAR: return GL_LINEAR;
            }
            return GL_LINEAR;
        }
    }

    OpenGLTextureManager::OpenGLTextureManager()
        : logger(spdlog::default_logger()->clone("OpenGLTextureManager"))
    {
    }

    OpenGLTextureManager::~OpenGLTextureManager()
    {
        Destroy();
    }

    void OpenGLTextureManager::Initialize()
    {
        CreateDefaultTexture();
        logger->info("OpenGLTextureManager initialized");
    }

    void OpenGLTextureManager::Destroy()
    {
        defaultTexture.reset();
    }

    std::unique_ptr<ITextureHandle> OpenGLTextureManager::CreateFromMemory(
        const TextureDescription& description, const void* pixels)
    {
        GLuint textureId = 0;
        glGenTextures(1, &textureId);

        if (textureId == 0)
        {
            logger->error("Failed to generate OpenGL texture");
            return nullptr;
        }

        glBindTexture(GL_TEXTURE_2D, textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGLWrapMode(description.wrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGLWrapMode(description.wrapT));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ToGLFilter(description.minFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ToGLFilter(description.magFilter));

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
            static_cast<GLsizei>(description.width),
            static_cast<GLsizei>(description.height),
            0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        glBindTexture(GL_TEXTURE_2D, 0);

        logger->info("Created OpenGL texture {}x{} (id={})", description.width, description.height, textureId);

        return std::make_unique<OpenGLTextureHandle>(textureId, description.width, description.height);
    }

    ITextureHandle* OpenGLTextureManager::GetDefaultTexture()
    {
        return defaultTexture.get();
    }

    void OpenGLTextureManager::CreateDefaultTexture()
    {
        uint32_t white = 0xFFFFFFFF;

        TextureDescription desc;
        desc.width = 1;
        desc.height = 1;
        desc.minFilter = TextureFilterMode::NEAREST;
        desc.magFilter = TextureFilterMode::NEAREST;

        defaultTexture = CreateFromMemory(desc, &white);
    }
}
