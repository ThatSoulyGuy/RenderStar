#pragma once

#include "RenderStar/Client/Render/Platform/IRenderingPlatformBackend.hpp"
#include <memory>

namespace RenderStar::Client::Render
{
    class IRenderBackend;
}

namespace RenderStar::Client::Render::Platform
{
    class PlatformBackendFactory
    {
    public:

        static std::unique_ptr<IRenderingPlatformBackend> Create(IRenderBackend* backend);
    };
}
