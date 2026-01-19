#pragma once

#include <cstdint>

namespace RenderStar::Client::Render
{
    enum class RenderBackend : int32_t
    {
        OPENGL,
        VULKAN
    };
}
