#pragma once

#include "RenderStar/Client/Render/Resource/ITextureManager.hpp"
#include <spdlog/spdlog.h>
#include <memory>

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLTextureManager : public ITextureManager
    {
    public:

        OpenGLTextureManager();
        ~OpenGLTextureManager() override;

        void Initialize();
        void Destroy();

        std::unique_ptr<ITextureHandle> CreateFromMemory(
            const TextureDescription& description, const void* pixels) override;

        ITextureHandle* GetDefaultTexture() override;

    private:

        void CreateDefaultTexture();

        std::shared_ptr<spdlog::logger> logger;
        std::unique_ptr<ITextureHandle> defaultTexture;
    };
}
