#pragma once

#include <cstdint>
#include <compare>

namespace RenderStar::Common::Component
{
    struct GameObject
    {
        int32_t id;

        static constexpr int32_t INVALID_ID = -1;

        static constexpr GameObject Invalid()
        {
            return { INVALID_ID };
        }

        constexpr bool IsValid() const noexcept
        {
            return id >= 0;
        }

        constexpr auto operator<=>(const GameObject&) const = default;
    };
}
