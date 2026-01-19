#include "RenderStar/Client/Render/Components/MeshRenderer.hpp"

namespace RenderStar::Client::Render::Components
{
    MeshRenderer MeshRenderer::Create()
    {
        MeshRenderer renderer;
        renderer.meshHandle = INVALID_MESH;
        renderer.materialHandle = INVALID_MATERIAL;
        renderer.visible = true;
        renderer.castShadows = true;
        renderer.receiveShadows = true;
        return renderer;
    }

    MeshRenderer MeshRenderer::Create(MeshHandle mesh, MaterialHandle material)
    {
        MeshRenderer renderer;
        renderer.meshHandle = mesh;
        renderer.materialHandle = material;
        renderer.visible = true;
        renderer.castShadows = true;
        renderer.receiveShadows = true;
        return renderer;
    }

    bool MeshRenderer::IsValid() const
    {
        return meshHandle != INVALID_MESH && materialHandle != INVALID_MATERIAL;
    }
}
