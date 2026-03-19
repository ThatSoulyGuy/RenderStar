#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <string>

namespace RenderStar::Client::UI
{
    struct UibinColor
    {
        float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;

        glm::vec4 ToVec4() const { return {r, g, b, a}; }

        static UibinColor Parse(const std::string& hex)
        {
            if (hex.size() != 9 || hex[0] != '#')
                return {1.0f, 1.0f, 1.0f, 1.0f};

            auto val = std::stoul(hex.substr(1), nullptr, 16);
            auto ca = static_cast<uint8_t>((val >> 24) & 0xFF);
            auto cr = static_cast<uint8_t>((val >> 16) & 0xFF);
            auto cg = static_cast<uint8_t>((val >>  8) & 0xFF);
            auto cb = static_cast<uint8_t>( val        & 0xFF);
            return {cr / 255.0f, cg / 255.0f, cb / 255.0f, ca / 255.0f};
        }
    };

    namespace AnchorFlags
    {
        constexpr uint32_t NONE      = 0x00;
        constexpr uint32_t LEFT      = 0x01;
        constexpr uint32_t RIGHT     = 0x02;
        constexpr uint32_t TOP       = 0x04;
        constexpr uint32_t BOTTOM    = 0x08;
        constexpr uint32_t CENTER_X  = 0x10;
        constexpr uint32_t CENTER_Y  = 0x20;
    }
}
