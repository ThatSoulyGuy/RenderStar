#include "RenderStar/Client/Render/OpenGL/OpenGLPlatformBackend.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLRenderTarget.hpp"
#include "RenderStar/Client/Render/OpenGL/OpenGLTextureHandle.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Shader/GlslTransformer.hpp"
#include <glad/gl.h>

namespace RenderStar::Client::Render::OpenGL
{
    OpenGLPlatformBackend::OpenGLPlatformBackend(IRenderBackend* backend)
        : logger(spdlog::default_logger()->clone("OpenGLPlatformBackend"))
        , backend(backend)
    {
        logger->info("OpenGL platform backend created");
    }

    std::unique_ptr<Platform::IRenderTarget> OpenGLPlatformBackend::CreateRenderTarget(
        const Platform::RenderTargetDescription& description)
    {
        auto desc = description;

        if (desc.matchSwapchainSize)
        {
            desc.width = backend->GetWidth();
            desc.height = backend->GetHeight();
        }

        return std::make_unique<OpenGLRenderTarget>(desc);
    }

    std::unique_ptr<Platform::IRenderTarget> OpenGLPlatformBackend::CreateSwapchainTarget()
    {
        return std::make_unique<OpenGLSwapchainTarget>(backend->GetWidth(), backend->GetHeight());
    }

    std::unique_ptr<IShaderProgram> OpenGLPlatformBackend::CompileShader(
        const std::string& vertexGlsl,
        const std::string& fragmentGlsl)
    {
        std::string transformedVertex = Shader::GlslTransformer::Transform450To410(vertexGlsl, Shader::ShaderType::VERTEX);
        std::string transformedFragment = Shader::GlslTransformer::Transform450To410(fragmentGlsl, Shader::ShaderType::FRAGMENT);

        ShaderSource source;
        source.vertexSource = transformedVertex;
        source.fragmentSource = transformedFragment;

        return backend->GetShaderManager()->CreateFromSource(source);
    }

    std::unique_ptr<IShaderProgram> OpenGLPlatformBackend::CompileComputeShader(
        const std::string& computeGlsl)
    {
        ShaderSource source;
        source.computeSource = computeGlsl;

        return backend->GetShaderManager()->CreateFromSource(source);
    }

    void OpenGLPlatformBackend::BindInputTextures(
        const std::vector<Platform::IRenderTarget*>& inputs,
        IShaderProgram*,
        int32_t)
    {
        for (uint32_t i = 0; i < inputs.size(); i++)
        {
            auto* colorAttachment = inputs[i]->GetColorAttachment(0);

            if (!colorAttachment)
                continue;

            auto* glTexture = static_cast<OpenGLTextureHandle*>(colorAttachment);
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, glTexture->GetTextureId());
        }
    }

    void OpenGLPlatformBackend::BeginRenderTarget(Platform::IRenderTarget* target, bool clear)
    {
        if (target->IsSwapchain())
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, static_cast<GLsizei>(target->GetWidth()), static_cast<GLsizei>(target->GetHeight()));
        }
        else
        {
            auto* glTarget = static_cast<OpenGLRenderTarget*>(target);
            glTarget->Bind();
        }

        if (clear)
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
    }

    void OpenGLPlatformBackend::EndRenderTarget(Platform::IRenderTarget* target)
    {
        if (target && !target->IsSwapchain())
        {
            auto* glTarget = static_cast<OpenGLRenderTarget*>(target);
            glTarget->Unbind();
        }
    }

    void OpenGLPlatformBackend::BlitToScreen(Platform::IRenderTarget* source)
    {
        if (source->IsSwapchain())
            return;

        auto* glSource = static_cast<OpenGLRenderTarget*>(source);
        uint32_t srcFbo = glSource->GetFramebufferHandle();
        uint32_t w = source->GetWidth();
        uint32_t h = source->GetHeight();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, static_cast<GLint>(w), static_cast<GLint>(h),
            0, 0, static_cast<GLint>(backend->GetWidth()), static_cast<GLint>(backend->GetHeight()),
            GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    IBufferManager* OpenGLPlatformBackend::GetBufferManager()
    {
        return backend->GetBufferManager();
    }

    IUniformManager* OpenGLPlatformBackend::GetUniformManager()
    {
        return backend->GetUniformManager();
    }

    ITextureManager* OpenGLPlatformBackend::GetTextureManager()
    {
        return backend->GetTextureManager();
    }

    IRenderCommandQueue* OpenGLPlatformBackend::GetCommandQueue()
    {
        return backend->GetCommandQueue();
    }

    void OpenGLPlatformBackend::SubmitDrawCommand(
        IShaderProgram* shader,
        IUniformBindingHandle* uniformBinding,
        int32_t frameIndex,
        IMesh* mesh)
    {
        backend->SubmitDrawCommand(shader, uniformBinding, frameIndex, mesh);
    }

    void OpenGLPlatformBackend::ExecuteDrawCommands()
    {
        backend->ExecuteDrawCommands();
    }

    void OpenGLPlatformBackend::OnResize(uint32_t, uint32_t)
    {
    }

    uint32_t OpenGLPlatformBackend::GetWidth() const
    {
        return backend->GetWidth();
    }

    uint32_t OpenGLPlatformBackend::GetHeight() const
    {
        return backend->GetHeight();
    }

    int32_t OpenGLPlatformBackend::GetCurrentFrame() const
    {
        return backend->GetCurrentFrame();
    }

    int32_t OpenGLPlatformBackend::GetMaxFramesInFlight() const
    {
        return backend->GetMaxFramesInFlight();
    }
}
