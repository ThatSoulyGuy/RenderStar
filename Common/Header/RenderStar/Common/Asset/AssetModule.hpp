#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Common/Asset/AssetLocation.hpp"
#include "RenderStar/Common/Asset/AssetHandle.hpp"
#include "RenderStar/Common/Asset/AssetCache.hpp"
#include "RenderStar/Common/Asset/IAssetProvider.hpp"
#include "RenderStar/Common/Asset/IAssetLoader.hpp"
#include "RenderStar/Common/Asset/ITextAsset.hpp"
#include "RenderStar/Common/Asset/IBinaryAsset.hpp"
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <filesystem>
#include <mutex>

namespace RenderStar::Common::Asset
{
    template <typename T>
    concept IAssetType = std::derived_from<T, IAsset>;

    class AssetModule final : public Module::AbstractModule
    {

    public:

        AssetModule();
        explicit AssetModule(std::filesystem::path  basePath);

        void RegisterProvider(std::unique_ptr<IAssetProvider> provider);
        void UnregisterProvider(std::string_view namespaceId);

        template <IAssetType T>
        void RegisterLoader(std::unique_ptr<IAssetLoader<T>> loader);

        template <IAssetType T>
        AssetHandle<T> Load(const AssetLocation& location);

        AssetHandle<ITextAsset> LoadText(const AssetLocation& location);
        AssetHandle<IBinaryAsset> LoadBinary(const AssetLocation& location);

        bool Exists(const AssetLocation& location) const;

        std::vector<AssetLocation> List(std::string_view namespaceId, std::string_view pathPrefix) const;
        std::vector<std::string> GetRegisteredNamespaces() const;

        void ClearCache();
        AssetCache& GetCache();

        const std::filesystem::path& GetBasePath() const;
        void SetBasePath(const std::filesystem::path& path);

    protected:

        void OnInitialize(Module::ModuleContext& context) override;

    private:

        void RegisterDefaultLoaders();
        void RegisterDefaultProvider();

        IAssetProvider* GetProviderForNamespace(std::string_view namespaceId) const;

        std::unordered_map<std::string, std::unique_ptr<IAssetProvider>> providers;

        AssetCache cache;

        std::unordered_map<std::type_index, std::shared_ptr<void>> loaders;
        std::filesystem::path basePath;

        mutable std::mutex moduleMutex;
    };

    template <IAssetType T>
    void AssetModule::RegisterLoader(std::unique_ptr<IAssetLoader<T>> loader)
    {
        std::lock_guard lock(moduleMutex);
        loaders[std::type_index(typeid(T))] = std::move(loader);
    }

    template <IAssetType T>
    AssetHandle<T> AssetModule::Load(const AssetLocation& location)
    {
        std::lock_guard lock(moduleMutex);

        if (auto cachedAsset = cache.Get<T>(location))
            return AssetHandle<T>(cachedAsset);

        auto provider = GetProviderForNamespace(location.GetNamespace());

        if (!provider)
            return AssetHandle<T>();

        const auto loaderIterator = loaders.find(std::type_index(typeid(T)));

        if (loaderIterator == loaders.end())
            return AssetHandle<T>();

        auto loader = std::static_pointer_cast<IAssetLoader<T>>(loaderIterator->second);
        auto asset = loader->Load(location, *provider);

        if (asset)
            cache.Put(location, asset);

        return AssetHandle<T>(asset);
    }
}
