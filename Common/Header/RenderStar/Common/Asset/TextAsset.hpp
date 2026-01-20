#pragma once

#include "RenderStar/Common/Asset/ITextAsset.hpp"

namespace RenderStar::Common::Asset
{
    class TextAsset : public ITextAsset
    {
    public:
        TextAsset(const AssetLocation& location, std::string content);

        const AssetLocation& GetLocation() const override;
        bool IsLoaded() const override;

        const std::string& GetContent() const override;
        std::string_view GetContentView() const override;

    private:
        AssetLocation location;
        std::string content;
        bool loaded;
    };
}
