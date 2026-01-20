#pragma once

#include <memory>

namespace RenderStar::Common::Asset
{
    template<typename T>
    class AssetHandle
    {
    public:
        AssetHandle() = default;

        explicit AssetHandle(std::shared_ptr<T> asset)
            : asset(std::move(asset))
        {
        }

        T* operator->() const
        {
            return asset.get();
        }

        T& operator*() const
        {
            return *asset;
        }

        T* Get() const
        {
            return asset.get();
        }

        bool IsValid() const
        {
            return asset != nullptr;
        }

        explicit operator bool() const
        {
            return IsValid();
        }

        void Release()
        {
            asset.reset();
        }

        long UseCount() const
        {
            return asset.use_count();
        }

    private:
        std::shared_ptr<T> asset;
    };
}
