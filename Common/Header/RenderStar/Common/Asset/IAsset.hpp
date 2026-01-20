#pragma once

#include "RenderStar/Common/Asset/AssetLocation.hpp"

namespace RenderStar::Common::Asset
{
    class IAsset
    {
    public:
        virtual ~IAsset() = default;

        virtual const AssetLocation& GetLocation() const = 0;
        virtual bool IsLoaded() const = 0;
    };
}
