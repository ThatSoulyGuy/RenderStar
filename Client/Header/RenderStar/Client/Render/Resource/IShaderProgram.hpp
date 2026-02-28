#pragma once

#include "RenderStar/Client/Render/Resource/IGraphicsResource.hpp"

namespace RenderStar::Client::Render
{
    class IShaderProgram : public IGraphicsResource
    {
    public:

        virtual bool IsValid() const = 0;
    };
}
