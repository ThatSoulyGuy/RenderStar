#pragma once

#include "RenderStar/Client/Render/Resource/IGraphicsResourceManager.hpp"
#include <spdlog/spdlog.h>
#include <mutex>
#include <unordered_set>

namespace RenderStar::Client::Render::Vulkan
{
    class VulkanGraphicsResourceManager : public IGraphicsResourceManager
    {
    public:

        VulkanGraphicsResourceManager();

        void Track(IGraphicsResource* resource) override;
        void Untrack(IGraphicsResource* resource) override;
        void ReleaseAll() override;
        size_t GetLiveResourceCount() const override;

    private:

        std::shared_ptr<spdlog::logger> logger;
        mutable std::mutex mutex;
        std::unordered_set<IGraphicsResource*> resources;
    };
}
