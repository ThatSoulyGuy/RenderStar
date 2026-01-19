#pragma once

#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include "RenderStar/Client/Render/Resource/IBufferHandle.hpp"
#include <cstdint>
#include <cstddef>

namespace RenderStar::Client::Render
{
    enum class PrimitiveType
    {
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
        LINES,
        LINE_STRIP,
        POINTS
    };

    enum class IndexType
    {
        UINT16,
        UINT32
    };

    class IMesh
    {
    public:

        virtual ~IMesh() = default;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual void Draw() = 0;
        virtual void DrawInstanced(int32_t instanceCount) = 0;

        virtual void SetVertexData(const void* data, size_t size) = 0;
        virtual void SetIndexData(const void* data, size_t size, IndexType indexType) = 0;

        virtual int32_t GetVertexCount() const = 0;
        virtual int32_t GetIndexCount() const = 0;

        virtual const VertexLayout& GetVertexLayout() const = 0;
        virtual PrimitiveType GetPrimitiveType() const = 0;

        virtual bool HasIndices() const = 0;
        virtual bool IsValid() const = 0;
    };
}
