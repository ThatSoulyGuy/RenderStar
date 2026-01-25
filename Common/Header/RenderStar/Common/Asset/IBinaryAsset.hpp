#pragma once

#include "RenderStar/Common/Asset/IAsset.hpp"
#include <vector>
#include <cstdint>
#include <span>

namespace RenderStar::Common::Asset
{
    class IBinaryAsset : public IAsset
    {

    public:

        ~IBinaryAsset() override = default;

        [[nodiscard]]
        virtual const std::vector<uint8_t>& GetData() const = 0;

        [[nodiscard]]
        virtual std::span<const uint8_t> GetDataView() const = 0;

        [[nodiscard]]
        virtual size_t GetSize() const = 0;
    };
}
