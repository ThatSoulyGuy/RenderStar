#pragma once

#include "RenderStar/Client/Render/Resource/IMesh.hpp"
#include "RenderStar/Client/Render/Resource/VertexLayout.hpp"
#include <cstdint>
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLMeshAdapter : public IMesh
    {
    public:

        OpenGLMeshAdapter(const VertexLayout& layout, PrimitiveType primitive);

        ~OpenGLMeshAdapter() override;

        void Bind();
        void Unbind();

        void Draw();
        void DrawInstanced(int32_t instanceCount);

        void SetVertexData(const void* data, size_t size) override;
        void SetIndexData(const void* data, size_t size, IndexType indexType) override;

        int32_t GetVertexCount() const override;
        int32_t GetIndexCount() const override;

        const VertexLayout& GetVertexLayout() const override;
        PrimitiveType GetPrimitiveType() const override;

        bool HasIndices() const override;
        bool IsValid() const override;

    private:

        void SetupVertexAttributes();
        uint32_t ToGLPrimitive(PrimitiveType type) const;
        uint32_t ToGLType(VertexAttributeType type) const;

        std::shared_ptr<spdlog::logger> logger;
        VertexLayout layout;
        PrimitiveType primitive;
        uint32_t vaoHandle;
        uint32_t vertexBuffer;
        uint32_t indexBuffer;
        int32_t vertexCount;
        int32_t indexCount;
        IndexType currentIndexType;
        bool hasIndices;
        bool valid;
    };
}
