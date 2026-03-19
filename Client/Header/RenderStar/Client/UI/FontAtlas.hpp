#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace RenderStar::Client::Render
{
    class ITextureManager;
    class ITextureHandle;
}

namespace RenderStar::Client::UI
{
    struct GlyphInfo
    {
        float uvX0, uvY0, uvX1, uvY1;
        float bearingX, bearingY;
        float advance;
        float width, height;
    };

    class FontAtlas
    {
    public:

        bool Build(const std::vector<uint8_t>& ttfData, float pixelSize, Render::ITextureManager* textureManager);

        const GlyphInfo* GetGlyph(uint32_t codepoint) const;
        Render::ITextureHandle* GetTexture() const;
        float GetLineHeight() const;
        float GetVisibleHeight() const;

    private:

        std::unordered_map<uint32_t, GlyphInfo> glyphs;
        std::unique_ptr<Render::ITextureHandle> atlasTexture;
        float lineHeight = 0.0f;
        float visibleHeight = 0.0f;
    };
}
