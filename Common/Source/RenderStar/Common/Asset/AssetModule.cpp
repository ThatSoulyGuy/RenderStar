#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Asset/FilesystemAssetProvider.hpp"
#include "RenderStar/Common/Asset/TextAssetLoader.hpp"
#include "RenderStar/Common/Asset/BinaryAssetLoader.hpp"
#include <spdlog/spdlog.h>

namespace RenderStar::Common::Asset
{
    AssetModule::AssetModule()
        : basePath(std::filesystem::current_path())
    {
    }

    AssetModule::AssetModule(const std::filesystem::path& basePath)
        : basePath(basePath)
    {
    }

    void AssetModule::OnInitialize(Module::ModuleContext& context)
    {
        RegisterDefaultLoaders();
        RegisterDefaultProvider();
        spdlog::info("AssetModule initialized with base path: {}", basePath.string());
    }

    void AssetModule::RegisterDefaultLoaders()
    {
        RegisterLoader<ITextAsset>(std::make_unique<TextAssetLoader>());
        RegisterLoader<IBinaryAsset>(std::make_unique<BinaryAssetLoader>());
    }

    void AssetModule::RegisterDefaultProvider()
    {
        auto defaultProvider = std::make_unique<FilesystemAssetProvider>(
            AssetLocation::DEFAULT_NAMESPACE,
            basePath / "assets" / std::string(AssetLocation::DEFAULT_NAMESPACE)
        );
        providers[std::string(AssetLocation::DEFAULT_NAMESPACE)] = std::move(defaultProvider);
    }

    void AssetModule::RegisterProvider(std::unique_ptr<IAssetProvider> provider)
    {
        std::lock_guard<std::mutex> lock(moduleMutex);
        auto namespaceId = provider->GetNamespace();
        providers[namespaceId] = std::move(provider);
        spdlog::info("Registered asset provider for namespace: {}", namespaceId);
    }

    void AssetModule::UnregisterProvider(std::string_view namespaceId)
    {
        std::lock_guard<std::mutex> lock(moduleMutex);
        providers.erase(std::string(namespaceId));
        spdlog::info("Unregistered asset provider for namespace: {}", namespaceId);
    }

    AssetHandle<ITextAsset> AssetModule::LoadText(const AssetLocation& location)
    {
        return Load<ITextAsset>(location);
    }

    AssetHandle<IBinaryAsset> AssetModule::LoadBinary(const AssetLocation& location)
    {
        return Load<IBinaryAsset>(location);
    }

    bool AssetModule::Exists(const AssetLocation& location) const
    {
        std::lock_guard<std::mutex> lock(moduleMutex);
        auto provider = GetProviderForNamespace(location.GetNamespace());
        if (!provider)
            return false;
        return provider->Exists(location);
    }

    std::vector<AssetLocation> AssetModule::List(std::string_view namespaceId, std::string_view pathPrefix) const
    {
        std::lock_guard<std::mutex> lock(moduleMutex);
        auto provider = GetProviderForNamespace(namespaceId);
        if (!provider)
            return {};
        return provider->List(pathPrefix);
    }

    std::vector<std::string> AssetModule::GetRegisteredNamespaces() const
    {
        std::lock_guard<std::mutex> lock(moduleMutex);
        std::vector<std::string> namespaces;
        namespaces.reserve(providers.size());
        for (const auto& [namespaceId, provider] : providers)
            namespaces.push_back(namespaceId);
        return namespaces;
    }

    void AssetModule::ClearCache()
    {
        cache.Clear();
    }

    AssetCache& AssetModule::GetCache()
    {
        return cache;
    }

    const std::filesystem::path& AssetModule::GetBasePath() const
    {
        return basePath;
    }

    void AssetModule::SetBasePath(const std::filesystem::path& path)
    {
        std::lock_guard<std::mutex> lock(moduleMutex);
        basePath = path;

        providers.clear();
        RegisterDefaultProvider();
    }

    IAssetProvider* AssetModule::GetProviderForNamespace(std::string_view namespaceId) const
    {
        auto it = providers.find(std::string(namespaceId));
        if (it == providers.end())
            return nullptr;
        return it->second.get();
    }
}
