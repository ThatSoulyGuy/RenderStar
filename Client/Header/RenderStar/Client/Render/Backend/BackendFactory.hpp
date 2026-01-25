#pragma once

#include "RenderStar/Client/Render/Backend/RenderBackend.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include <memory>
#include <vector>

namespace RenderStar::Client::Render
{
    class BackendFactory
    {
    public:

        static void Initialize();

        static std::unique_ptr<IRenderBackend> Create(RenderBackend backendType);

        static RenderBackend DetectBestBackend();

        static bool IsBackendAvailable(RenderBackend backendType);

        static std::vector<RenderBackend> GetAvailableBackends();

    private:

        static bool initialized;
    };
}
