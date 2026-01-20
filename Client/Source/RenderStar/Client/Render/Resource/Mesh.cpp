#include "RenderStar/Client/Render/Resource/Mesh.hpp"

namespace RenderStar::Client::Render::Resource
{
    Mesh::Mesh(IBufferManager& bufferManager, const VertexLayout& layout, PrimitiveType primitive)
        : mesh(bufferManager.CreateMesh(layout, primitive))
    {
    }

    void Mesh::SetVertexData(const void* data, size_t size)
    {
        if (mesh)
            mesh->SetVertexData(data, size);
    }

    void Mesh::SetIndexData(const void* data, size_t size, IndexType indexType)
    {
        if (mesh)
            mesh->SetIndexData(data, size, indexType);
    }

    int32_t Mesh::GetVertexCount() const
    {
        return mesh ? mesh->GetVertexCount() : 0;
    }

    int32_t Mesh::GetIndexCount() const
    {
        return mesh ? mesh->GetIndexCount() : 0;
    }

    const VertexLayout& Mesh::GetVertexLayout() const
    {
        static const VertexLayout emptyLayout{};
        return mesh ? mesh->GetVertexLayout() : emptyLayout;
    }

    PrimitiveType Mesh::GetPrimitiveType() const
    {
        return mesh ? mesh->GetPrimitiveType() : PrimitiveType::TRIANGLES;
    }

    bool Mesh::HasIndices() const
    {
        return mesh && mesh->HasIndices();
    }

    bool Mesh::IsValid() const
    {
        return mesh && mesh->IsValid();
    }

    IMesh* Mesh::GetUnderlyingMesh()
    {
        return mesh.get();
    }

    const IMesh* Mesh::GetUnderlyingMesh() const
    {
        return mesh.get();
    }
}
