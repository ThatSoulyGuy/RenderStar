#pragma once

#include "RenderStar/Client/Render/Backend/RenderBackend.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include <memory>

namespace RenderStar::Client::Render
{
    class BackendFactory
    {
    public:

        static std::unique_ptr<IRenderBackend> Create(RenderBackend backendType);

        static RenderBackend DetectBestBackend();

        static bool IsBackendAvailable(RenderBackend backendType);
    };
}
