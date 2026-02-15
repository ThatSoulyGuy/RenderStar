#include <gtest/gtest.h>
#include "RenderStar/Common/Component/Components/Hierarchy.hpp"

using namespace RenderStar::Common::Component;

TEST(HierarchyTest, DefaultParentIsInvalid)
{
    Hierarchy hierarchy;
    EXPECT_EQ(hierarchy.parent.id, GameObject::INVALID_ID);
}

TEST(HierarchyTest, DefaultChildrenEmpty)
{
    Hierarchy hierarchy;
    EXPECT_TRUE(hierarchy.children.empty());
}

TEST(HierarchyTest, HasParentFalseByDefault)
{
    Hierarchy hierarchy;
    EXPECT_FALSE(hierarchy.HasParent());
}

TEST(HierarchyTest, HasParentTrueWhenSet)
{
    Hierarchy hierarchy;
    hierarchy.parent = GameObject{5};
    EXPECT_TRUE(hierarchy.HasParent());
}

TEST(HierarchyTest, AddAndRemoveChildren)
{
    Hierarchy hierarchy;
    hierarchy.children.push_back(GameObject{1});
    hierarchy.children.push_back(GameObject{2});
    EXPECT_EQ(hierarchy.children.size(), 2);

    hierarchy.children.erase(hierarchy.children.begin());
    EXPECT_EQ(hierarchy.children.size(), 1);
    EXPECT_EQ(hierarchy.children[0].id, 2);
}
