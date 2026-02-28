#pragma once

#include <cstdint>

namespace RenderStar::Client::Render::Resource
{
    class Mesh;
}

namespace RenderStar::Client::Render::Components
{
    struct MapbinMesh
    {
        Resource::Mesh* mesh = nullptr;
        int32_t materialId = 0;
    };
}
