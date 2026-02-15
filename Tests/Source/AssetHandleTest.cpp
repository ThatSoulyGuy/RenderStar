#include <gtest/gtest.h>
#include "RenderStar/Common/Asset/AssetHandle.hpp"
#include "RenderStar/Common/Asset/TextAsset.hpp"

using namespace RenderStar::Common::Asset;

TEST(AssetHandleTest, DefaultConstructionInvalid)
{
    AssetHandle<TextAsset> handle;
    EXPECT_FALSE(handle.IsValid());
    EXPECT_FALSE(static_cast<bool>(handle));
}

TEST(AssetHandleTest, ConstructWithSharedPtr)
{
    auto asset = std::make_shared<TextAsset>(AssetLocation::Of("test", "a"), "hello");
    AssetHandle<TextAsset> handle(asset);
    EXPECT_TRUE(handle.IsValid());
    EXPECT_TRUE(static_cast<bool>(handle));
}

TEST(AssetHandleTest, AccessUnderlyingAsset)
{
    auto asset = std::make_shared<TextAsset>(AssetLocation::Of("test", "a"), "hello");
    AssetHandle<TextAsset> handle(asset);

    EXPECT_EQ(handle->GetContent(), "hello");
    EXPECT_EQ((*handle).GetContent(), "hello");
    EXPECT_EQ(handle.Get()->GetContent(), "hello");
}

TEST(AssetHandleTest, ReleaseInvalidates)
{
    auto asset = std::make_shared<TextAsset>(AssetLocation::Of("test", "a"), "hello");
    AssetHandle<TextAsset> handle(asset);
    EXPECT_TRUE(handle.IsValid());

    handle.Release();
    EXPECT_FALSE(handle.IsValid());
}

TEST(AssetHandleTest, UseCount)
{
    auto asset = std::make_shared<TextAsset>(AssetLocation::Of("test", "a"), "hello");
    AssetHandle<TextAsset> handle(asset);
    EXPECT_EQ(handle.UseCount(), 2);
}
