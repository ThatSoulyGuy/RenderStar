#pragma once

#include "RenderStar/Common/Asset/IBinaryAsset.hpp"

namespace RenderStar::Common::Asset
{
    class BinaryAsset : public IBinaryAsset
    {
    public:
        BinaryAsset(const AssetLocation& location, std::vector<uint8_t> data);

        const AssetLocation& GetLocation() const override;
        bool IsLoaded() const override;

        const std::vector<uint8_t>& GetData() const override;
        std::span<const uint8_t> GetDataView() const override;
        size_t GetSize() const override;

    private:
        AssetLocation location;
        std::vector<uint8_t> data;
        bool loaded;
    };
}
