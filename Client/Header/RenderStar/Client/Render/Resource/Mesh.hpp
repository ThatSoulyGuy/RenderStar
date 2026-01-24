#pragma once

#include "RenderStar/Client/Render/Resource/IMesh.hpp"
#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Client/Render/Resource/IVertex.hpp"
#include "RenderStar/Client/Render/Resource/Vertex.hpp"
#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include <memory>
#include <vector>
#include <span>
#include <concepts>

namespace RenderStar::Client::Render::Resource
{
    class Mesh
    {
    public:
        Mesh(IBufferManager& bufferManager, const VertexLayout& layout, PrimitiveType primitive = PrimitiveType::TRIANGLES);

        template<typename VertexType>
        requires std::derived_from<VertexType, IVertex>
        void SetVertices(std::span<const VertexType> vertices);

        template<typename VertexType>
        requires std::derived_from<VertexType, IVertex>
        void SetVertices(const std::vector<VertexType>& vertices);

        void SetVertexData(const void* data, size_t size) const;

        template<typename IndexT>
        requires std::integral<IndexT>
        void SetIndices(std::span<const IndexT> indices);

        template<typename IndexT>
        requires std::integral<IndexT>
        void SetIndices(const std::vector<IndexT>& indices);

        void SetIndexData(const void* data, size_t size, IndexType indexType) const;

        int32_t GetVertexCount() const;
        int32_t GetIndexCount() const;
        const VertexLayout& GetVertexLayout() const;
        PrimitiveType GetPrimitiveType() const;
        bool HasIndices() const;
        bool IsValid() const;

        IMesh* GetUnderlyingMesh();
        const IMesh* GetUnderlyingMesh() const;

    private:
        std::unique_ptr<IMesh> mesh;
    };

    template<typename VertexType>
    requires std::derived_from<VertexType, IVertex>
    void Mesh::SetVertices(std::span<const VertexType> vertices)
    {
        if (!mesh || vertices.empty())
            return;

        std::vector<float> floatData = Vertex::ToFloatArray(vertices);
        mesh->SetVertexData(floatData.data(), floatData.size() * sizeof(float));
    }

    template<typename VertexType>
    requires std::derived_from<VertexType, IVertex>
    void Mesh::SetVertices(const std::vector<VertexType>& vertices)
    {
        SetVertices(std::span<const VertexType>(vertices));
    }

    template<typename IndexT>
    requires std::integral<IndexT>
    void Mesh::SetIndices(std::span<const IndexT> indices)
    {
        if (!mesh || indices.empty())
            return;

        IndexType indexType;
        if constexpr (sizeof(IndexT) == 2)
            indexType = IndexType::UINT16;
        else
            indexType = IndexType::UINT32;

        mesh->SetIndexData(indices.data(), indices.size() * sizeof(IndexT), indexType);
    }

    template<typename IndexT>
    requires std::integral<IndexT>
    void Mesh::SetIndices(const std::vector<IndexT>& indices)
    {
        SetIndices(std::span<const IndexT>(indices));
    }
}
