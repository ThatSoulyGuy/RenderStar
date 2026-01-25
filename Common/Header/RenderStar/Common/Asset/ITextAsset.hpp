#pragma once

#include "RenderStar/Common/Asset/IAsset.hpp"
#include <string>
#include <string_view>

namespace RenderStar::Common::Asset
{
    class ITextAsset : public IAsset
    {

    public:

        ~ITextAsset() override = default;

        [[nodiscard]]
        virtual const std::string& GetContent() const = 0;

        [[nodiscard]]
        virtual std::string_view GetContentView() const = 0;
    };
}
