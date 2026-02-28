#pragma once

#include <cstdint>

namespace RenderStar::Client::Render
{
    class IGraphicsResourceManager;

    enum class GraphicsResourceType : int32_t
    {
        UNIFORM_BINDING,
        SHADER_PROGRAM,
        MESH,
        TEXTURE,
        BUFFER
    };

    class IGraphicsResource
    {
    public:

        virtual ~IGraphicsResource();

        virtual void Release() = 0;

        virtual GraphicsResourceType GetResourceType() const = 0;

        bool IsReleased() const { return released; }

    protected:

        IGraphicsResourceManager* resourceManager = nullptr;
        bool released = false;

        friend class IGraphicsResourceManager;
    };
}
