#include "RenderStar/Client/Render/Vulkan/VulkanBufferManager.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanBufferHandle.hpp"
#include "RenderStar/Client/Render/Vulkan/VulkanMesh.hpp"

namespace RenderStar::Client::Render::Vulkan
{
    VulkanBufferManager::VulkanBufferManager()
        : logger(spdlog::default_logger()->clone("VulkanBufferManager"))
        , bufferModule(nullptr)
    {
    }

    VulkanBufferManager::~VulkanBufferManager() = default;

    void VulkanBufferManager::Initialize(VulkanBufferModule* module)
    {
        bufferModule = module;
        logger->info("Vulkan buffer manager initialized");
    }

    std::unique_ptr<IBufferHandle> VulkanBufferManager::CreateBuffer(
        BufferType type,
        BufferUsage usage,
        size_t size,
        const void* initialData)
    {
        VulkanBufferType vulkanType = ToVulkanBufferType(type);
        bool hostVisible = IsHostVisible(usage);

        VulkanBuffer buffer = bufferModule->CreateBuffer(vulkanType, size, hostVisible);

        auto handle = std::make_unique<VulkanBufferHandle>(bufferModule, buffer, type, usage);

        if (initialData)
            handle->SetData(initialData, size);

        return handle;
    }

    std::unique_ptr<IBufferHandle> VulkanBufferManager::CreateVertexBuffer(
        size_t size,
        const void* data,
        BufferUsage usage)
    {
        return CreateBuffer(BufferType::VERTEX, usage, size, data);
    }

    std::unique_ptr<IBufferHandle> VulkanBufferManager::CreateIndexBuffer(
        size_t size,
        const void* data,
        BufferUsage usage)
    {
        return CreateBuffer(BufferType::INDEX, usage, size, data);
    }

    std::unique_ptr<IBufferHandle> VulkanBufferManager::CreateUniformBuffer(
        size_t size,
        BufferUsage usage)
    {
        return CreateBuffer(BufferType::UNIFORM, usage, size, nullptr);
    }

    std::unique_ptr<IMesh> VulkanBufferManager::CreateMesh(
        const VertexLayout& layout,
        PrimitiveType primitive)
    {
        auto mesh = std::make_unique<VulkanMesh>();
        mesh->Initialize(bufferModule, layout, primitive);
        return mesh;
    }

    void VulkanBufferManager::DestroyBuffer(IBufferHandle* buffer)
    {
        (void)buffer;
    }

    VulkanBufferType VulkanBufferManager::ToVulkanBufferType(BufferType type) const
    {
        switch (type)
        {
            case BufferType::VERTEX:
                return VulkanBufferType::VERTEX;
            case BufferType::INDEX:
                return VulkanBufferType::INDEX;
            case BufferType::UNIFORM:
                return VulkanBufferType::UNIFORM;
            case BufferType::STORAGE:
                return VulkanBufferType::STORAGE;
        }

        return VulkanBufferType::VERTEX;
    }

    bool VulkanBufferManager::IsHostVisible(BufferUsage usage) const
    {
        return usage == BufferUsage::DYNAMIC || usage == BufferUsage::STREAM;
    }
}
