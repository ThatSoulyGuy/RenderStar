#include "RenderStar/Common/Asset/BinaryAsset.hpp"

namespace RenderStar::Common::Asset
{
    BinaryAsset::BinaryAsset(const AssetLocation& location, std::vector<uint8_t> data)
        : location(location)
        , data(std::move(data))
        , loaded(true)
    {
    }

    const AssetLocation& BinaryAsset::GetLocation() const
    {
        return location;
    }

    bool BinaryAsset::IsLoaded() const
    {
        return loaded;
    }

    const std::vector<uint8_t>& BinaryAsset::GetData() const
    {
        return data;
    }

    std::span<const uint8_t> BinaryAsset::GetDataView() const
    {
        return data;
    }

    size_t BinaryAsset::GetSize() const
    {
        return data.size();
    }
}
