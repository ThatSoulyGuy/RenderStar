#pragma once

#include "RenderStar/Common/Asset/AssetLocation.hpp"

namespace RenderStar::Common::Asset
{
    class IAsset
    {

    public:

        virtual ~IAsset() = default;

        [[nodiscard]]
        virtual const AssetLocation& GetLocation() const = 0;

        [[nodiscard]]
        virtual bool IsLoaded() const = 0;
    };
}
