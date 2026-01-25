#pragma once

#include "RenderStar/Common/Asset/ITextAsset.hpp"

namespace RenderStar::Common::Asset
{
    class TextAsset final : public ITextAsset
    {

    public:

        TextAsset(AssetLocation  location, std::string content);

        [[nodiscard]]
        const AssetLocation& GetLocation() const override;

        [[nodiscard]]
        bool IsLoaded() const override;

        [[nodiscard]]
        const std::string& GetContent() const override;

        [[nodiscard]]
        std::string_view GetContentView() const override;

    private:

        AssetLocation location;
        std::string content;

        bool loaded;

    };
}
