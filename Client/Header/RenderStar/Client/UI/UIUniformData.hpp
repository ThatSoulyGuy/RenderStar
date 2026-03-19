#pragma once

#include <cstddef>

namespace RenderStar::Client::UI
{
    struct UIUniformData
    {
        float posX, posY, width, height;
        float u0, v0, u1, v1;
        float tintR, tintG, tintB, tintA;
        float screenW, screenH, rotation, cornerRadius;

        static constexpr size_t Size() { return sizeof(UIUniformData); }
    };
}
