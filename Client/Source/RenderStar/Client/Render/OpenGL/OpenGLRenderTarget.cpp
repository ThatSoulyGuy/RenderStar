#include "RenderStar/Client/Render/OpenGL/OpenGLRenderTarget.hpp"
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    namespace
    {
        GLenum ToGLInternalFormat(TextureFormat format)
        {
            switch (format)
            {
            case TextureFormat::RGBA8: return GL_RGBA8;
            case TextureFormat::RGBA16F: return GL_RGBA16F;
            case TextureFormat::RGBA32F: return GL_RGBA32F;
            default: return GL_RGBA8;
            }
        }

        GLenum ToGLFormat(TextureFormat format)
        {
            switch (format)
            {
            case TextureFormat::RGBA8: return GL_RGBA;
            case TextureFormat::RGBA16F: return GL_RGBA;
            case TextureFormat::RGBA32F: return GL_RGBA;
            default: return GL_RGBA;
            }
        }

        GLenum ToGLType(TextureFormat format)
        {
            switch (format)
            {
            case TextureFormat::RGBA8: return GL_UNSIGNED_BYTE;
            case TextureFormat::RGBA16F: return GL_FLOAT;
            case TextureFormat::RGBA32F: return GL_FLOAT;
            default: return GL_UNSIGNED_BYTE;
            }
        }
    }

    OpenGLRenderTarget::OpenGLRenderTarget(const Platform::RenderTargetDescription& description)
        : logger(spdlog::default_logger()->clone("OpenGLRenderTarget"))
        , description(description)
    {
        Create();
    }

    OpenGLRenderTarget::~OpenGLRenderTarget()
    {
        Destroy();
    }

    const std::string& OpenGLRenderTarget::GetName() const
    {
        return description.name;
    }

    ITextureHandle* OpenGLRenderTarget::GetColorAttachment(uint32_t index) const
    {
        if (index == 0 && colorAttachment)
            return colorAttachment.get();

        return nullptr;
    }

    ITextureHandle* OpenGLRenderTarget::GetDepthAttachment() const
    {
        return depthAttachment.get();
    }

    uint32_t OpenGLRenderTarget::GetWidth() const
    {
        return description.width;
    }

    uint32_t OpenGLRenderTarget::GetHeight() const
    {
        return description.height;
    }

    void OpenGLRenderTarget::Resize(uint32_t width, uint32_t height)
    {
        if (width == description.width && height == description.height)
            return;

        description.width = width;
        description.height = height;

        Destroy();
        Create();

        logger->debug("Resized render target '{}' to {}x{}", description.name, width, height);
    }

    Platform::RenderTargetType OpenGLRenderTarget::GetType() const
    {
        if (description.hasDepth)
            return Platform::RenderTargetType::COLOR_DEPTH;

        return Platform::RenderTargetType::COLOR_ONLY;
    }

    bool OpenGLRenderTarget::IsSwapchain() const
    {
        return false;
    }

    uint32_t OpenGLRenderTarget::GetFramebufferHandle() const
    {
        return fbo;
    }

    uint32_t OpenGLRenderTarget::GetSampleCount() const
    {
        return description.sampleCount;
    }

    void OpenGLRenderTarget::Bind() const
    {
        if (msaaFbo != 0)
            glBindFramebuffer(GL_FRAMEBUFFER, msaaFbo);
        else
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glViewport(0, 0, static_cast<GLsizei>(description.width), static_cast<GLsizei>(description.height));
    }

    void OpenGLRenderTarget::Unbind() const
    {
        if (msaaFbo != 0)
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
            glBlitFramebuffer(
                0, 0, static_cast<GLint>(description.width), static_cast<GLint>(description.height),
                0, 0, static_cast<GLint>(description.width), static_cast<GLint>(description.height),
                GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLRenderTarget::Create()
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLuint colorTex = 0;
        glGenTextures(1, &colorTex);
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, ToGLInternalFormat(description.colorFormat),
            static_cast<GLsizei>(description.width),
            static_cast<GLsizei>(description.height),
            0, ToGLFormat(description.colorFormat), ToGLType(description.colorFormat), nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
        colorAttachment = std::make_unique<OpenGLTextureHandle>(colorTex, description.width, description.height);

        if (description.hasDepth)
        {
            GLuint depthTex = 0;
            glGenTextures(1, &depthTex);
            glBindTexture(GL_TEXTURE_2D, depthTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8,
                static_cast<GLsizei>(description.width),
                static_cast<GLsizei>(description.height),
                0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
            depthAttachment = std::make_unique<OpenGLTextureHandle>(depthTex, description.width, description.height);
        }

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE)
            logger->error("Framebuffer '{}' incomplete: 0x{:X}", description.name, status);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (description.sampleCount > 1)
        {
            GLsizei samples = static_cast<GLsizei>(description.sampleCount);

            glGenFramebuffers(1, &msaaFbo);
            glBindFramebuffer(GL_FRAMEBUFFER, msaaFbo);

            glGenRenderbuffers(1, &msaaColorRbo);
            glBindRenderbuffer(GL_RENDERBUFFER, msaaColorRbo);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                ToGLInternalFormat(description.colorFormat),
                static_cast<GLsizei>(description.width),
                static_cast<GLsizei>(description.height));
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorRbo);

            if (description.hasDepth)
            {
                glGenRenderbuffers(1, &msaaDepthRbo);
                glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthRbo);
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8,
                    static_cast<GLsizei>(description.width),
                    static_cast<GLsizei>(description.height));
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaaDepthRbo);
            }

            GLenum msaaStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

            if (msaaStatus != GL_FRAMEBUFFER_COMPLETE)
                logger->error("MSAA framebuffer '{}' incomplete: 0x{:X}", description.name, msaaStatus);
            else
                logger->info("Created render target '{}' {}x{} ({}x MSAA)", description.name, description.width, description.height, description.sampleCount);

            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        else
        {
            logger->info("Created render target '{}' {}x{} (fbo={})", description.name, description.width, description.height, fbo);
        }
    }

    void OpenGLRenderTarget::Destroy()
    {
        colorAttachment.reset();
        depthAttachment.reset();

        if (msaaDepthRbo != 0)
        {
            glDeleteRenderbuffers(1, &msaaDepthRbo);
            msaaDepthRbo = 0;
        }

        if (msaaColorRbo != 0)
        {
            glDeleteRenderbuffers(1, &msaaColorRbo);
            msaaColorRbo = 0;
        }

        if (msaaFbo != 0)
        {
            glDeleteFramebuffers(1, &msaaFbo);
            msaaFbo = 0;
        }

        if (fbo != 0)
        {
            glDeleteFramebuffers(1, &fbo);
            fbo = 0;
        }
    }

    OpenGLSwapchainTarget::OpenGLSwapchainTarget(uint32_t width, uint32_t height)
        : name("SWAPCHAIN")
        , width(width)
        , height(height)
    {
    }

    const std::string& OpenGLSwapchainTarget::GetName() const
    {
        return name;
    }

    ITextureHandle* OpenGLSwapchainTarget::GetColorAttachment(uint32_t) const
    {
        return nullptr;
    }

    ITextureHandle* OpenGLSwapchainTarget::GetDepthAttachment() const
    {
        return nullptr;
    }

    uint32_t OpenGLSwapchainTarget::GetWidth() const
    {
        return width;
    }

    uint32_t OpenGLSwapchainTarget::GetHeight() const
    {
        return height;
    }

    void OpenGLSwapchainTarget::Resize(uint32_t newWidth, uint32_t newHeight)
    {
        width = newWidth;
        height = newHeight;
    }

    Platform::RenderTargetType OpenGLSwapchainTarget::GetType() const
    {
        return Platform::RenderTargetType::SWAPCHAIN;
    }

    bool OpenGLSwapchainTarget::IsSwapchain() const
    {
        return true;
    }
}
