#pragma once

#include "RenderStar/Client/Render/Command/IRenderCommandBuffer.hpp"
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    class OpenGLShaderProgram;
    class OpenGLBufferHandle;
    class OpenGLMeshAdapter;

    class OpenGLCommandBuffer : public IRenderCommandBuffer
    {
    public:

        OpenGLCommandBuffer();
        ~OpenGLCommandBuffer() override;

        void Begin() override;
        void End() override;

        void BindPipeline(IShaderProgram* shader) override;
        void BindVertexBuffer(IBufferHandle* buffer, uint32_t binding) override;
        void BindIndexBuffer(IBufferHandle* buffer, IndexType type) override;
        void BindUniformSet(IUniformBindingHandle* uniforms, uint32_t set) override;

        void Draw(uint32_t vertexCount, uint32_t firstVertex) override;
        void DrawIndexed(uint32_t indexCount, uint32_t firstIndex) override;
        void DrawMesh(IMesh* mesh) override;

        void SetViewport(float x, float y, float width, float height) override;
        void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) override;

        void Reset() override;
        bool IsRecording() const override;

    private:

        std::shared_ptr<spdlog::logger> logger;
        bool recording;
        OpenGLShaderProgram* currentShader;
        uint32_t currentVao;
        IndexType currentIndexType;
    };
}
