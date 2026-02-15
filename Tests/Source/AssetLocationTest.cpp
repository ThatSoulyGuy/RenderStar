#include <gtest/gtest.h>
#include "RenderStar/Common/Asset/AssetLocation.hpp"
#include <unordered_map>

using namespace RenderStar::Common::Asset;

TEST(AssetLocationTest, ParseWithNamespace)
{
    auto loc = AssetLocation::Parse("renderstar:shader/test.vert");
    EXPECT_EQ(loc.GetNamespace(), "renderstar");
    EXPECT_EQ(loc.GetPath(), "shader/test.vert");
}

TEST(AssetLocationTest, ParseWithoutNamespace)
{
    auto loc = AssetLocation::Parse("shader/test.vert");
    EXPECT_EQ(loc.GetNamespace(), std::string(AssetLocation::DEFAULT_NAMESPACE));
    EXPECT_EQ(loc.GetPath(), "shader/test.vert");
}

TEST(AssetLocationTest, ParseCustomNamespace)
{
    auto loc = AssetLocation::Parse("custom:textures/brick.png");
    EXPECT_EQ(loc.GetNamespace(), "custom");
    EXPECT_EQ(loc.GetPath(), "textures/brick.png");
}

TEST(AssetLocationTest, OfFactoryMethod)
{
    auto loc = AssetLocation::Of("mymod", "models/cube.obj");
    EXPECT_EQ(loc.GetNamespace(), "mymod");
    EXPECT_EQ(loc.GetPath(), "models/cube.obj");
}

TEST(AssetLocationTest, ToStringRoundtrip)
{
    auto original = AssetLocation::Of("renderstar", "shader/test.vert");
    auto str = original.ToString();
    auto parsed = AssetLocation::Parse(str);
    EXPECT_EQ(parsed, original);
}

TEST(AssetLocationTest, ToFilesystemPath)
{
    auto loc = AssetLocation::Of("renderstar", "shader/test.vert");
    auto fsPath = loc.ToFilesystemPath("/base/resources");
    EXPECT_FALSE(fsPath.empty());
}

TEST(AssetLocationTest, IsValidTrue)
{
    auto loc = AssetLocation::Of("ns", "path");
    EXPECT_TRUE(loc.IsValid());
}

TEST(AssetLocationTest, IsValidFalseForDefault)
{
    AssetLocation loc;
    EXPECT_FALSE(loc.IsValid());
}

TEST(AssetLocationTest, Equality)
{
    auto a = AssetLocation::Of("ns", "path");
    auto b = AssetLocation::Of("ns", "path");
    auto c = AssetLocation::Of("other", "path");

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(AssetLocationTest, HashableInMap)
{
    std::unordered_map<AssetLocation, int> map;
    auto loc = AssetLocation::Of("ns", "test");
    map[loc] = 42;
    EXPECT_EQ(map[loc], 42);
}

TEST(AssetLocationTest, SpaceshipOrdering)
{
    auto a = AssetLocation::Of("a", "path");
    auto b = AssetLocation::Of("b", "path");
    EXPECT_TRUE(a < b);
}

TEST(AssetLocationTest, NamespaceSeparatorConstant)
{
    EXPECT_EQ(AssetLocation::NAMESPACE_SEPARATOR, ':');
}
