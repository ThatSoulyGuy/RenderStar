#include <gtest/gtest.h>
#include "RenderStar/Common/Asset/AssetCache.hpp"
#include "RenderStar/Common/Asset/TextAsset.hpp"

using namespace RenderStar::Common::Asset;

class AssetCacheTest : public ::testing::Test
{
protected:
    AssetCache cache{4};

    std::shared_ptr<TextAsset> MakeAsset(const std::string& name)
    {
        return std::make_shared<TextAsset>(AssetLocation::Of("test", name), "content of " + name);
    }
};

TEST_F(AssetCacheTest, PutAndGet)
{
    auto loc = AssetLocation::Of("test", "a");
    auto asset = MakeAsset("a");
    cache.Put<TextAsset>(loc, asset);

    auto retrieved = cache.Get<TextAsset>(loc);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->GetContent(), "content of a");
}

TEST_F(AssetCacheTest, GetMissingReturnsNull)
{
    auto loc = AssetLocation::Of("test", "nonexistent");
    EXPECT_EQ(cache.Get<TextAsset>(loc), nullptr);
}

TEST_F(AssetCacheTest, EvictRemovesEntry)
{
    auto loc = AssetLocation::Of("test", "a");
    cache.Put<TextAsset>(loc, MakeAsset("a"));
    cache.Evict(loc);
    EXPECT_EQ(cache.Get<TextAsset>(loc), nullptr);
}

TEST_F(AssetCacheTest, ClearRemovesAll)
{
    cache.Put<TextAsset>(AssetLocation::Of("test", "a"), MakeAsset("a"));
    cache.Put<TextAsset>(AssetLocation::Of("test", "b"), MakeAsset("b"));
    cache.Clear();
    EXPECT_EQ(cache.GetSize(), 0);
}

TEST_F(AssetCacheTest, LruEviction)
{
    auto locA = AssetLocation::Of("test", "a");
    auto locB = AssetLocation::Of("test", "b");
    auto locC = AssetLocation::Of("test", "c");
    auto locD = AssetLocation::Of("test", "d");
    auto locE = AssetLocation::Of("test", "e");

    cache.Put<TextAsset>(locA, MakeAsset("a"));
    cache.Put<TextAsset>(locB, MakeAsset("b"));
    cache.Put<TextAsset>(locC, MakeAsset("c"));
    cache.Put<TextAsset>(locD, MakeAsset("d"));
    cache.Put<TextAsset>(locE, MakeAsset("e"));

    EXPECT_EQ(cache.GetSize(), 4);
}

TEST_F(AssetCacheTest, ExpiredWeakPtrCleanedOnGet)
{
    auto loc = AssetLocation::Of("test", "a");

    {
        auto asset = MakeAsset("a");
        cache.Put<TextAsset>(loc, asset);
    }

    auto retrieved = cache.Get<TextAsset>(loc);
    EXPECT_EQ(retrieved, nullptr);
}

TEST_F(AssetCacheTest, SetMaxSize)
{
    cache.SetMaxSize(2);
    EXPECT_EQ(cache.GetMaxSize(), 2);
}

TEST_F(AssetCacheTest, GetSizeAndMaxSize)
{
    EXPECT_EQ(cache.GetSize(), 0);
    EXPECT_EQ(cache.GetMaxSize(), 4);

    cache.Put<TextAsset>(AssetLocation::Of("test", "a"), MakeAsset("a"));
    EXPECT_EQ(cache.GetSize(), 1);
}

TEST_F(AssetCacheTest, RePutSameKeyUpdates)
{
    auto loc = AssetLocation::Of("test", "a");
    auto first = MakeAsset("first");
    auto second = MakeAsset("second");

    cache.Put<TextAsset>(loc, first);
    cache.Put<TextAsset>(loc, second);

    auto retrieved = cache.Get<TextAsset>(loc);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->GetContent(), "content of second");
}

TEST_F(AssetCacheTest, DefaultCacheMaxSize)
{
    AssetCache defaultCache;
    EXPECT_EQ(defaultCache.GetMaxSize(), AssetCache::DEFAULT_MAX_SIZE);
}
