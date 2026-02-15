#include <gtest/gtest.h>
#include "RenderStar/Common/Asset/TextAsset.hpp"

using namespace RenderStar::Common::Asset;

TEST(TextAssetTest, ConstructWithContent)
{
    TextAsset asset(AssetLocation::Of("test", "file.txt"), "hello world");
    EXPECT_EQ(asset.GetContent(), "hello world");
}

TEST(TextAssetTest, GetContentView)
{
    TextAsset asset(AssetLocation::Of("test", "file.txt"), "content");
    EXPECT_EQ(asset.GetContentView(), "content");
}

TEST(TextAssetTest, EmptyContent)
{
    TextAsset asset(AssetLocation::Of("test", "empty.txt"), "");
    EXPECT_TRUE(asset.GetContent().empty());
}

TEST(TextAssetTest, LocationPreserved)
{
    auto loc = AssetLocation::Of("test", "file.txt");
    TextAsset asset(loc, "data");
    EXPECT_EQ(asset.GetLocation(), loc);
}

TEST(TextAssetTest, IsLoaded)
{
    TextAsset asset(AssetLocation::Of("test", "file.txt"), "data");
    EXPECT_TRUE(asset.IsLoaded());
}
