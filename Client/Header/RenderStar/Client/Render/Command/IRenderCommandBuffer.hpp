#pragma once

#include <cstdint>

namespace RenderStar::Client::Render
{
    class IShaderProgram;
    class IBufferHandle;
    class IUniformBindingHandle;
    class IMesh;
    enum class IndexType;

    class IRenderCommandBuffer
    {
    public:

        virtual ~IRenderCommandBuffer() = default;

        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual void BindPipeline(IShaderProgram* shader) = 0;
        virtual void BindVertexBuffer(IBufferHandle* buffer, uint32_t binding) = 0;
        virtual void BindIndexBuffer(IBufferHandle* buffer, IndexType type) = 0;
        virtual void BindUniformSet(IUniformBindingHandle* uniforms, uint32_t set) = 0;

        virtual void Draw(uint32_t vertexCount, uint32_t firstVertex) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t firstIndex) = 0;
        virtual void DrawMesh(IMesh* mesh) = 0;

        virtual void SetViewport(float x, float y, float width, float height) = 0;
        virtual void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) = 0;

        virtual void Reset() = 0;
        virtual bool IsRecording() const = 0;
    };
}
