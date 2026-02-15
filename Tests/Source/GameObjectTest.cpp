#include <gtest/gtest.h>
#include "RenderStar/Common/Component/GameObject.hpp"
#include <unordered_map>

using namespace RenderStar::Common::Component;

TEST(GameObjectTest, DefaultConstructionIsInvalid)
{
    GameObject entity{};
    EXPECT_EQ(entity.id, 0);
}

TEST(GameObjectTest, ExplicitIdConstruction)
{
    GameObject entity{42};
    EXPECT_EQ(entity.id, 42);
}

TEST(GameObjectTest, InvalidIdConstant)
{
    EXPECT_EQ(GameObject::INVALID_ID, -1);
}

TEST(GameObjectTest, InvalidFactoryReturnsInvalid)
{
    auto entity = GameObject::Invalid();
    EXPECT_EQ(entity.id, GameObject::INVALID_ID);
    EXPECT_FALSE(entity.IsValid());
}

TEST(GameObjectTest, IsValidForPositiveId)
{
    GameObject entity{0};
    EXPECT_TRUE(entity.IsValid());

    GameObject entity2{100};
    EXPECT_TRUE(entity2.IsValid());
}

TEST(GameObjectTest, IsValidFalseForNegativeId)
{
    GameObject entity{-1};
    EXPECT_FALSE(entity.IsValid());

    GameObject entity2{-42};
    EXPECT_FALSE(entity2.IsValid());
}

TEST(GameObjectTest, EqualityOperator)
{
    GameObject a{5};
    GameObject b{5};
    GameObject c{10};

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(GameObjectTest, SpaceshipOrdering)
{
    GameObject a{1};
    GameObject b{2};

    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a <= a);
}

TEST(GameObjectTest, HashableInUnorderedMap)
{
    std::unordered_map<int32_t, std::string> map;
    GameObject entity{7};
    map[entity.id] = "test";

    EXPECT_EQ(map[7], "test");
}

TEST(GameObjectTest, CopySemantics)
{
    GameObject original{42};
    GameObject copy = original;

    EXPECT_EQ(copy.id, 42);
    EXPECT_EQ(original, copy);
}
