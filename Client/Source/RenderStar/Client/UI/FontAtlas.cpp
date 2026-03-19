#include "RenderStar/Client/UI/FontAtlas.hpp"
#include "RenderStar/Client/Render/Resource/ITextureManager.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace RenderStar::Client::UI
{
    bool FontAtlas::Build(const std::vector<uint8_t>& ttfData, float pixelSize, Render::ITextureManager* textureManager)
    {
        if (ttfData.empty() || !textureManager)
            return false;

        stbtt_fontinfo fontInfo;
        if (!stbtt_InitFont(&fontInfo, ttfData.data(), stbtt_GetFontOffsetForIndex(ttfData.data(), 0)))
            return false;

        float scale = stbtt_ScaleForPixelHeight(&fontInfo, pixelSize);

        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
        lineHeight = (ascent - descent + lineGap) * scale;
        visibleHeight = (ascent - descent) * scale;

        constexpr int ATLAS_SIZE = 512;
        std::vector<uint8_t> atlasBitmap(ATLAS_SIZE * ATLAS_SIZE, 0);

        stbtt_pack_context packCtx;
        if (!stbtt_PackBegin(&packCtx, atlasBitmap.data(), ATLAS_SIZE, ATLAS_SIZE, 0, 1, nullptr))
            return false;

        stbtt_PackSetOversampling(&packCtx, 2, 2);

        constexpr int FIRST_CHAR = 32;
        constexpr int NUM_CHARS = 95;
        std::vector<stbtt_packedchar> charData(NUM_CHARS);

        stbtt_PackFontRange(&packCtx, ttfData.data(), 0, pixelSize,
            FIRST_CHAR, NUM_CHARS, charData.data());

        stbtt_PackEnd(&packCtx);

        // Build glyph info using screen-space dimensions from stb_truetype
        for (int i = 0; i < NUM_CHARS; ++i)
        {
            const auto& pc = charData[i];
            uint32_t codepoint = static_cast<uint32_t>(FIRST_CHAR + i);

            GlyphInfo info;
            info.uvX0 = static_cast<float>(pc.x0) / ATLAS_SIZE;
            info.uvY0 = static_cast<float>(pc.y0) / ATLAS_SIZE;
            info.uvX1 = static_cast<float>(pc.x1) / ATLAS_SIZE;
            info.uvY1 = static_cast<float>(pc.y1) / ATLAS_SIZE;
            // Use xoff2-xoff / yoff2-yoff for correct screen-space dimensions
            // (x1-x0 / y1-y0 are atlas bitmap pixels, inflated by oversampling)
            info.width = pc.xoff2 - pc.xoff;
            info.height = pc.yoff2 - pc.yoff;
            info.bearingX = pc.xoff;
            info.bearingY = pc.yoff;
            info.advance = pc.xadvance;

            glyphs[codepoint] = info;
        }

        // Convert single-channel to RGBA
        std::vector<uint8_t> rgbaData(ATLAS_SIZE * ATLAS_SIZE * 4);
        for (int i = 0; i < ATLAS_SIZE * ATLAS_SIZE; ++i)
        {
            rgbaData[i * 4 + 0] = 255;
            rgbaData[i * 4 + 1] = 255;
            rgbaData[i * 4 + 2] = 255;
            rgbaData[i * 4 + 3] = atlasBitmap[i];
        }

        Render::TextureDescription desc;
        desc.width = ATLAS_SIZE;
        desc.height = ATLAS_SIZE;
        desc.format = Render::TextureFormat::RGBA8;
        desc.wrapS = Render::TextureWrapMode::CLAMP_TO_EDGE;
        desc.wrapT = Render::TextureWrapMode::CLAMP_TO_EDGE;
        desc.minFilter = Render::TextureFilterMode::LINEAR;
        desc.magFilter = Render::TextureFilterMode::LINEAR;
        desc.generateMipmaps = false;

        atlasTexture = textureManager->CreateFromMemory(desc, rgbaData.data());

        return atlasTexture && atlasTexture->IsValid();
    }

    const GlyphInfo* FontAtlas::GetGlyph(uint32_t codepoint) const
    {
        auto it = glyphs.find(codepoint);
        return it != glyphs.end() ? &it->second : nullptr;
    }

    Render::ITextureHandle* FontAtlas::GetTexture() const
    {
        return atlasTexture.get();
    }

    float FontAtlas::GetLineHeight() const
    {
        return lineHeight;
    }

    float FontAtlas::GetVisibleHeight() const
    {
        return visibleHeight;
    }
}
