#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <spdlog/spdlog.h>

namespace RenderStar::Client::Render::OpenGL
{
    using VertexArrayHandle = uint32_t;

    struct VertexAttribute
    {
        uint32_t location;
        int32_t componentCount;
        uint32_t type;
        bool normalized;
        int32_t stride;
        size_t offset;
    };

    class OpenGLMesh
    {
    public:

        static constexpr VertexArrayHandle INVALID_VAO = 0;

        OpenGLMesh();

        ~OpenGLMesh();

        void Create(const std::vector<VertexAttribute>& attributes);

        void Destroy();

        void Bind();

        void Unbind();

        void SetVertexBuffer(uint32_t buffer);

        void SetIndexBuffer(uint32_t buffer);

        void Draw(int32_t indexCount);

        void DrawInstanced(int32_t indexCount, int32_t instanceCount);

        VertexArrayHandle GetHandle() const;

        bool IsValid() const;

    private:

        std::shared_ptr<spdlog::logger> logger;
        VertexArrayHandle vaoHandle;
        uint32_t vertexBuffer;
        uint32_t indexBuffer;
        std::vector<VertexAttribute> attributes;
    };
}
