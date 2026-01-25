#pragma once

#include "RenderStar/Common/Asset/IBinaryAsset.hpp"

namespace RenderStar::Common::Asset
{
    class BinaryAsset final : public IBinaryAsset
    {

    public:

        BinaryAsset(AssetLocation  location, std::vector<uint8_t> data);

        [[nodiscard]]
        const AssetLocation& GetLocation() const override;

        [[nodiscard]]
        bool IsLoaded() const override;

        [[nodiscard]]
        const std::vector<uint8_t>& GetData() const override;

        [[nodiscard]]
        std::span<const uint8_t> GetDataView() const override;

        [[nodiscard]]
        size_t GetSize() const override;

    private:

        AssetLocation location;
        std::vector<uint8_t> data;

        bool loaded;
    };
}
