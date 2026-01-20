#include "RenderStar/Client/Render/Backend/BackendFactory.hpp"
#include <unordered_map>
#include <algorithm>

namespace RenderStar::Client::Render
{
    struct BackendEntry
    {
        BackendCreator creator;
        BackendAvailabilityChecker checker;
        int32_t priority;
    };

    static std::unordered_map<RenderBackend, BackendEntry>& GetRegistry()
    {
        static std::unordered_map<RenderBackend, BackendEntry> registry;
        return registry;
    }

    void BackendFactory::RegisterBackend(
        RenderBackend type,
        BackendCreator creator,
        BackendAvailabilityChecker checker,
        int32_t priority)
    {
        GetRegistry()[type] = BackendEntry{ std::move(creator), std::move(checker), priority };
    }

    std::unique_ptr<IRenderBackend> BackendFactory::Create(RenderBackend backendType)
    {
        auto& registry = GetRegistry();
        auto it = registry.find(backendType);
        if (it != registry.end() && it->second.creator)
            return it->second.creator();

        return nullptr;
    }

    RenderBackend BackendFactory::DetectBestBackend()
    {
        auto& registry = GetRegistry();
        RenderBackend best = RenderBackend::OPENGL;
        int32_t bestPriority = -1;

        for (const auto& [type, entry] : registry)
        {
            if (entry.checker && entry.checker() && entry.priority > bestPriority)
            {
                best = type;
                bestPriority = entry.priority;
            }
        }

        return best;
    }

    bool BackendFactory::IsBackendAvailable(RenderBackend backendType)
    {
        auto& registry = GetRegistry();
        auto it = registry.find(backendType);
        if (it != registry.end() && it->second.checker)
            return it->second.checker();

        return false;
    }

    std::vector<RenderBackend> BackendFactory::GetAvailableBackends()
    {
        std::vector<RenderBackend> result;
        auto& registry = GetRegistry();

        for (const auto& [type, entry] : registry)
        {
            if (entry.checker && entry.checker())
                result.push_back(type);
        }

        return result;
    }
}
