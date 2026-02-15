#include <gtest/gtest.h>
#include "RenderStar/Common/Asset/BinaryAsset.hpp"

using namespace RenderStar::Common::Asset;

TEST(BinaryAssetTest, ConstructWithData)
{
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    BinaryAsset asset(AssetLocation::Of("test", "file.bin"), data);
    EXPECT_EQ(asset.GetData().size(), 3);
    EXPECT_EQ(asset.GetData()[0], 0x01);
    EXPECT_EQ(asset.GetData()[1], 0x02);
    EXPECT_EQ(asset.GetData()[2], 0x03);
}

TEST(BinaryAssetTest, GetDataView)
{
    std::vector<uint8_t> data = {0xAA, 0xBB};
    BinaryAsset asset(AssetLocation::Of("test", "file.bin"), data);
    auto view = asset.GetDataView();
    EXPECT_EQ(view.size(), 2);
}

TEST(BinaryAssetTest, EmptyData)
{
    BinaryAsset asset(AssetLocation::Of("test", "empty.bin"), {});
    EXPECT_TRUE(asset.GetData().empty());
    EXPECT_EQ(asset.GetSize(), 0);
}

TEST(BinaryAssetTest, LocationAndSize)
{
    auto loc = AssetLocation::Of("test", "file.bin");
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    BinaryAsset asset(loc, data);
    EXPECT_EQ(asset.GetLocation(), loc);
    EXPECT_EQ(asset.GetSize(), 5);
    EXPECT_TRUE(asset.IsLoaded());
}
