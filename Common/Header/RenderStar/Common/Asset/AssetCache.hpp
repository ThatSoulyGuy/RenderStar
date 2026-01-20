#pragma once

#include "RenderStar/Common/Asset/AssetLocation.hpp"
#include "RenderStar/Common/Asset/IAsset.hpp"
#include <unordered_map>
#include <list>
#include <memory>
#include <mutex>

namespace RenderStar::Common::Asset
{
    class AssetCache
    {
    public:
        static constexpr size_t DEFAULT_MAX_SIZE = 256;

        AssetCache();
        explicit AssetCache(size_t maxSize);

        template<typename T>
        std::shared_ptr<T> Get(const AssetLocation& location);

        template<typename T>
        void Put(const AssetLocation& location, std::shared_ptr<T> asset);

        void Evict(const AssetLocation& location);
        void Clear();
        void SetMaxSize(size_t size);
        size_t GetSize() const;
        size_t GetMaxSize() const;

    private:
        void TouchEntry(const AssetLocation& location);
        void EvictLeastRecentlyUsed();

        std::unordered_map<AssetLocation, std::weak_ptr<IAsset>> cache;
        std::list<AssetLocation> lruOrder;
        std::unordered_map<AssetLocation, std::list<AssetLocation>::iterator> lruIterators;
        size_t maxCacheSize;
        mutable std::mutex cacheMutex;
    };

    template<typename T>
    std::shared_ptr<T> AssetCache::Get(const AssetLocation& location)
    {
        std::lock_guard<std::mutex> lock(cacheMutex);

        auto it = cache.find(location);
        if (it == cache.end())
            return nullptr;

        auto sharedAsset = it->second.lock();
        if (!sharedAsset)
        {
            cache.erase(it);
            auto lruIt = lruIterators.find(location);
            if (lruIt != lruIterators.end())
            {
                lruOrder.erase(lruIt->second);
                lruIterators.erase(lruIt);
            }
            return nullptr;
        }

        TouchEntry(location);
        return std::dynamic_pointer_cast<T>(sharedAsset);
    }

    template<typename T>
    void AssetCache::Put(const AssetLocation& location, std::shared_ptr<T> asset)
    {
        std::lock_guard<std::mutex> lock(cacheMutex);

        auto existingIt = cache.find(location);
        if (existingIt != cache.end())
        {
            existingIt->second = asset;
            TouchEntry(location);
            return;
        }

        while (cache.size() >= maxCacheSize)
            EvictLeastRecentlyUsed();

        cache[location] = asset;
        lruOrder.push_front(location);
        lruIterators[location] = lruOrder.begin();
    }
}
