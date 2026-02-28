#include "RenderStar/Client/Render/Vulkan/VulkanGraphicsResourceManager.hpp"
#include "RenderStar/Client/Render/Resource/IGraphicsResource.hpp"
#include <array>
#include <vector>

namespace RenderStar::Client::Render::Vulkan
{
    VulkanGraphicsResourceManager::VulkanGraphicsResourceManager()
        : logger(spdlog::default_logger()->clone("VulkanGraphicsResourceManager"))
    {
    }

    void VulkanGraphicsResourceManager::Track(IGraphicsResource* resource)
    {
        if (!resource)
            return;

        std::lock_guard lock(mutex);
        resources.insert(resource);
        AttachResource(resource, this);
    }

    void VulkanGraphicsResourceManager::Untrack(IGraphicsResource* resource)
    {
        if (!resource)
            return;

        std::lock_guard lock(mutex);
        resources.erase(resource);
    }

    void VulkanGraphicsResourceManager::ReleaseAll()
    {
        std::lock_guard lock(mutex);

        if (resources.empty())
            return;

        size_t totalCount = resources.size();

        static constexpr std::array releaseOrder =
        {
            GraphicsResourceType::UNIFORM_BINDING,
            GraphicsResourceType::SHADER_PROGRAM,
            GraphicsResourceType::MESH,
            GraphicsResourceType::TEXTURE,
            GraphicsResourceType::BUFFER
        };

        for (GraphicsResourceType type : releaseOrder)
        {
            std::vector<IGraphicsResource*> batch;

            for (IGraphicsResource* resource : resources)
            {
                if (resource->GetResourceType() == type && !resource->IsReleased())
                    batch.push_back(resource);
            }

            for (IGraphicsResource* resource : batch)
                resource->Release();
        }

        for (IGraphicsResource* resource : resources)
            DetachResource(resource);

        resources.clear();

        logger->info("ReleaseAll: force-released {} GPU resources", totalCount);
    }

    size_t VulkanGraphicsResourceManager::GetLiveResourceCount() const
    {
        std::lock_guard lock(mutex);
        return resources.size();
    }
}
