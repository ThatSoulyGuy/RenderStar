#pragma once

#include "RenderStar/Client/Render/Backend/RenderBackend.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include <memory>
#include <functional>
#include <vector>

namespace RenderStar::Client::Render
{
    using BackendCreator = std::function<std::unique_ptr<IRenderBackend>()>;
    using BackendAvailabilityChecker = std::function<bool()>;

    class BackendFactory
    {
    public:

        static void RegisterBackend(
            RenderBackend type,
            BackendCreator creator,
            BackendAvailabilityChecker checker,
            int32_t priority = 0);

        static std::unique_ptr<IRenderBackend> Create(RenderBackend backendType);

        static RenderBackend DetectBestBackend();

        static bool IsBackendAvailable(RenderBackend backendType);

        static std::vector<RenderBackend> GetAvailableBackends();
    };
}
