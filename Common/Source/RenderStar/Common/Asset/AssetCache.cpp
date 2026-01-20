#include "RenderStar/Common/Asset/AssetCache.hpp"

namespace RenderStar::Common::Asset
{
    AssetCache::AssetCache()
        : maxCacheSize(DEFAULT_MAX_SIZE)
    {
    }

    AssetCache::AssetCache(size_t maxSize)
        : maxCacheSize(maxSize)
    {
    }

    void AssetCache::Evict(const AssetLocation& location)
    {
        std::lock_guard<std::mutex> lock(cacheMutex);

        auto it = cache.find(location);
        if (it != cache.end())
        {
            cache.erase(it);
            auto lruIt = lruIterators.find(location);
            if (lruIt != lruIterators.end())
            {
                lruOrder.erase(lruIt->second);
                lruIterators.erase(lruIt);
            }
        }
    }

    void AssetCache::Clear()
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        cache.clear();
        lruOrder.clear();
        lruIterators.clear();
    }

    void AssetCache::SetMaxSize(size_t size)
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        maxCacheSize = size;

        while (cache.size() > maxCacheSize)
            EvictLeastRecentlyUsed();
    }

    size_t AssetCache::GetSize() const
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        return cache.size();
    }

    size_t AssetCache::GetMaxSize() const
    {
        return maxCacheSize;
    }

    void AssetCache::TouchEntry(const AssetLocation& location)
    {
        auto lruIt = lruIterators.find(location);
        if (lruIt != lruIterators.end())
        {
            lruOrder.erase(lruIt->second);
            lruOrder.push_front(location);
            lruIt->second = lruOrder.begin();
        }
    }

    void AssetCache::EvictLeastRecentlyUsed()
    {
        if (lruOrder.empty())
            return;

        auto& lruLocation = lruOrder.back();
        cache.erase(lruLocation);
        lruIterators.erase(lruLocation);
        lruOrder.pop_back();
    }
}
