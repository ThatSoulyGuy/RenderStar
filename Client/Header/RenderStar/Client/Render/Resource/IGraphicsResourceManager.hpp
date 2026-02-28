#pragma once

#include "RenderStar/Client/Render/Resource/IGraphicsResource.hpp"

#include <cstddef>

namespace RenderStar::Client::Render
{
    class IGraphicsResourceManager
    {
    public:

        virtual ~IGraphicsResourceManager() = default;

        virtual void Track(IGraphicsResource* resource) = 0;
        virtual void Untrack(IGraphicsResource* resource) = 0;
        virtual void ReleaseAll() = 0;
        virtual size_t GetLiveResourceCount() const = 0;

    protected:

        static void AttachResource(IGraphicsResource* resource, IGraphicsResourceManager* manager)
        {
            resource->resourceManager = manager;
        }

        static void DetachResource(IGraphicsResource* resource)
        {
            resource->resourceManager = nullptr;
        }
    };
}
