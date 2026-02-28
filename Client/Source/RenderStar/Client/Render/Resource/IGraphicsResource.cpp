#include "RenderStar/Client/Render/Resource/IGraphicsResource.hpp"
#include "RenderStar/Client/Render/Resource/IGraphicsResourceManager.hpp"

namespace RenderStar::Client::Render
{
    IGraphicsResource::~IGraphicsResource()
    {
        if (resourceManager)
            resourceManager->Untrack(this);
    }
}
