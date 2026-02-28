#pragma once

#include <cstdint>

namespace RenderStar::Client::Render
{
    enum class TextureFormat
    {
        RGBA8
    };

    enum class TextureWrapMode
    {
        REPEAT,
        CLAMP_TO_EDGE
    };

    enum class TextureFilterMode
    {
        NEAREST,
        LINEAR
    };

    struct TextureDescription
    {
        uint32_t width = 1;
        uint32_t height = 1;
        TextureFormat format = TextureFormat::RGBA8;
        TextureWrapMode wrapS = TextureWrapMode::REPEAT;
        TextureWrapMode wrapT = TextureWrapMode::REPEAT;
        TextureFilterMode minFilter = TextureFilterMode::LINEAR;
        TextureFilterMode magFilter = TextureFilterMode::LINEAR;
    };

    class ITextureHandle
    {
    public:

        virtual ~ITextureHandle() = default;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual bool IsValid() const = 0;
    };
}
