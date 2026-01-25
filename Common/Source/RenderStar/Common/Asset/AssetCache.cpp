#include "RenderStar/Common/Asset/AssetCache.hpp"

namespace RenderStar::Common::Asset
{
    AssetCache::AssetCache() : maxCacheSize(DEFAULT_MAX_SIZE) { }

    AssetCache::AssetCache(const size_t maxSize) : maxCacheSize(maxSize) { }

    void AssetCache::Evict(const AssetLocation& location)
    {
        std::lock_guard lock(cacheMutex);

        if (const auto iterator = cache.find(location); iterator != cache.end())
        {
            cache.erase(iterator);

            if (const auto lruIt = lruIterators.find(location); lruIt != lruIterators.end())
            {
                lruOrder.erase(lruIt->second);
                lruIterators.erase(lruIt);
            }
        }
    }

    void AssetCache::Clear()
    {
        std::lock_guard lock(cacheMutex);
        cache.clear();
        lruOrder.clear();
        lruIterators.clear();
    }

    void AssetCache::SetMaxSize(const size_t size)
    {
        std::lock_guard lock(cacheMutex);
        maxCacheSize = size;

        while (cache.size() > maxCacheSize)
            EvictLeastRecentlyUsed();
    }

    size_t AssetCache::GetSize() const
    {
        std::lock_guard lock(cacheMutex);
        return cache.size();
    }

    size_t AssetCache::GetMaxSize() const
    {
        return maxCacheSize;
    }

    void AssetCache::TouchEntry(const AssetLocation& location)
    {
        if (const auto lruIt = lruIterators.find(location); lruIt != lruIterators.end())
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

        const auto& lruLocation = lruOrder.back();

        cache.erase(lruLocation);
        lruIterators.erase(lruLocation);
        lruOrder.pop_back();
    }
}
