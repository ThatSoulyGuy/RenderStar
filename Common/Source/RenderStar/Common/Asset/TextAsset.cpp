#include "RenderStar/Common/Asset/TextAsset.hpp"

namespace RenderStar::Common::Asset
{
    TextAsset::TextAsset(const AssetLocation& location, std::string content)
        : location(location)
        , content(std::move(content))
        , loaded(true)
    {
    }

    const AssetLocation& TextAsset::GetLocation() const
    {
        return location;
    }

    bool TextAsset::IsLoaded() const
    {
        return loaded;
    }

    const std::string& TextAsset::GetContent() const
    {
        return content;
    }

    std::string_view TextAsset::GetContentView() const
    {
        return content;
    }
}
