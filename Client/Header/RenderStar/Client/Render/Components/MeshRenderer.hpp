#pragma once

#include <cstdint>
#include <string>

namespace RenderStar::Client::Render::Components
{
    using MeshHandle = int32_t;
    using MaterialHandle = int32_t;

    struct MeshRenderer
    {
        MeshHandle meshHandle;
        MaterialHandle materialHandle;
        bool visible;
        bool castShadows;
        bool receiveShadows;

        static constexpr MeshHandle INVALID_MESH = -1;
        static constexpr MaterialHandle INVALID_MATERIAL = -1;

        static MeshRenderer Create();

        static MeshRenderer Create(MeshHandle mesh, MaterialHandle material);

        bool IsValid() const;
    };
}
