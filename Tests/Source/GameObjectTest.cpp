#include <gtest/gtest.h>
#include "RenderStar/Common/Component/GameObject.hpp"

using namespace RenderStar::Common::Component;

class GameObjectTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
    }
};

TEST_F(GameObjectTest, DefaultConstructionIsInvalid)
{
    GameObject object{GameObject::INVALID_ID};
    EXPECT_FALSE(object.IsValid());
}

TEST_F(GameObjectTest, ValidIdIsValid)
{
    GameObject object{0};
    EXPECT_TRUE(object.IsValid());

    GameObject object2{100};
    EXPECT_TRUE(object2.IsValid());
}

TEST_F(GameObjectTest, NegativeIdIsInvalid)
{
    GameObject object{-1};
    EXPECT_FALSE(object.IsValid());

    GameObject object2{-100};
    EXPECT_FALSE(object2.IsValid());
}

TEST_F(GameObjectTest, InvalidFactoryMethod)
{
    GameObject object = GameObject::Invalid();
    EXPECT_FALSE(object.IsValid());
    EXPECT_EQ(object.id, GameObject::INVALID_ID);
}

TEST_F(GameObjectTest, EqualityComparison)
{
    GameObject a{5};
    GameObject b{5};
    GameObject c{10};

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST_F(GameObjectTest, OrderingComparison)
{
    GameObject a{5};
    GameObject b{10};
    GameObject c{5};

    EXPECT_LT(a, b);
    EXPECT_GT(b, a);
    EXPECT_LE(a, c);
    EXPECT_GE(b, a);
}

TEST_F(GameObjectTest, InvalidIdConstant)
{
    EXPECT_EQ(GameObject::INVALID_ID, -1);
}

TEST_F(GameObjectTest, CanBeUsedInContainer)
{
    std::vector<GameObject> objects;
    objects.push_back(GameObject{0});
    objects.push_back(GameObject{1});
    objects.push_back(GameObject{2});

    EXPECT_EQ(objects.size(), 3);
    EXPECT_EQ(objects[0].id, 0);
    EXPECT_EQ(objects[1].id, 1);
    EXPECT_EQ(objects[2].id, 2);
}

TEST_F(GameObjectTest, CanBeUsedAsMapKey)
{
    std::map<GameObject, int32_t> objectMap;
    objectMap[GameObject{0}] = 100;
    objectMap[GameObject{5}] = 500;

    EXPECT_EQ(objectMap[GameObject{0}], 100);
    EXPECT_EQ(objectMap[GameObject{5}], 500);
}
