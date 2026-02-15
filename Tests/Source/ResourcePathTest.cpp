#include <gtest/gtest.h>
#include "RenderStar/Common/Utility/ResourcePath.hpp"

using namespace RenderStar::Common::Utility;

TEST(ResourcePathTest, DefaultConstruction)
{
    ResourcePath path;
    EXPECT_TRUE(path.domain.empty());
    EXPECT_TRUE(path.localPath.empty());
}

TEST(ResourcePathTest, ParameterizedConstruction)
{
    ResourcePath path("renderstar", "shader/test.vert");
    EXPECT_EQ(path.domain, "renderstar");
    EXPECT_EQ(path.localPath, "shader/test.vert");
}

TEST(ResourcePathTest, FullPath)
{
    ResourcePath path("renderstar", "shader/test.vert");
    EXPECT_EQ(path.FullPath(), "/assets/renderstar/shader/test.vert");
}

TEST(ResourcePathTest, RelativePath)
{
    ResourcePath path("renderstar", "shader/test.vert");
    EXPECT_EQ(path.RelativePath(), "assets/renderstar/shader/test.vert");
}

TEST(ResourcePathTest, ToFilesystemPath)
{
    ResourcePath path("renderstar", "shader/test.vert");
    auto fsPath = path.ToFilesystemPath("/base");
    auto expected = std::filesystem::path("/base") / "assets" / "renderstar" / "shader/test.vert";
    EXPECT_EQ(fsPath, expected);
}

TEST(ResourcePathTest, Equality)
{
    ResourcePath a("renderstar", "shader/test.vert");
    ResourcePath b("renderstar", "shader/test.vert");
    ResourcePath c("other", "shader/test.vert");

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(ResourcePathTest, EmptyDomainAndPath)
{
    ResourcePath path("", "");
    EXPECT_EQ(path.FullPath(), "/assets//");
    EXPECT_EQ(path.RelativePath(), "assets//");
}
