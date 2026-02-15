#include <gtest/gtest.h>
#include "RenderStar/Common/Component/ComponentPool.hpp"
#include "RenderStar/Common/Component/GameObject.hpp"

using namespace RenderStar::Common::Component;

struct TestComponent
{
    int32_t value;
    float multiplier;
};

class ComponentPoolTest : public ::testing::Test
{
protected:
    ComponentPool<TestComponent> pool;
};

TEST_F(ComponentPoolTest, AddAndHas)
{
    GameObject entity{0};
    pool.Add(entity, TestComponent{42, 1.5f});
    EXPECT_TRUE(pool.Has(entity));
}

TEST_F(ComponentPoolTest, GetReturnsCorrectValue)
{
    GameObject entity{0};
    pool.Add(entity, TestComponent{42, 1.5f});
    auto retrieved = pool.Get(entity);

    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->get().value, 42);
    EXPECT_FLOAT_EQ(retrieved->get().multiplier, 1.5f);
}

TEST_F(ComponentPoolTest, RemoveEntity)
{
    GameObject entity{0};
    pool.Add(entity, TestComponent{42, 1.5f});
    pool.Remove(entity);
    EXPECT_FALSE(pool.Has(entity));
}

TEST_F(ComponentPoolTest, GetNonExistentReturnsEmpty)
{
    auto retrieved = pool.Get(GameObject{99});
    EXPECT_FALSE(retrieved.has_value());
}

TEST_F(ComponentPoolTest, MultipleEntities)
{
    pool.Add(GameObject{0}, TestComponent{10, 1.0f});
    pool.Add(GameObject{1}, TestComponent{20, 2.0f});
    pool.Add(GameObject{2}, TestComponent{30, 3.0f});

    ASSERT_TRUE(pool.Get(GameObject{0}).has_value());
    ASSERT_TRUE(pool.Get(GameObject{1}).has_value());
    ASSERT_TRUE(pool.Get(GameObject{2}).has_value());

    EXPECT_EQ(pool.Get(GameObject{0})->get().value, 10);
    EXPECT_EQ(pool.Get(GameObject{1})->get().value, 20);
    EXPECT_EQ(pool.Get(GameObject{2})->get().value, 30);
}

TEST_F(ComponentPoolTest, RemoveMiddlePreservesOthers)
{
    pool.Add(GameObject{0}, TestComponent{10, 1.0f});
    pool.Add(GameObject{1}, TestComponent{20, 2.0f});
    pool.Add(GameObject{2}, TestComponent{30, 3.0f});

    pool.Remove(GameObject{1});

    EXPECT_TRUE(pool.Has(GameObject{0}));
    EXPECT_FALSE(pool.Has(GameObject{1}));
    EXPECT_TRUE(pool.Has(GameObject{2}));
    EXPECT_EQ(pool.Get(GameObject{0})->get().value, 10);
    EXPECT_EQ(pool.Get(GameObject{2})->get().value, 30);
}

TEST_F(ComponentPoolTest, IterateOverComponents)
{
    pool.Add(GameObject{0}, TestComponent{10, 1.0f});
    pool.Add(GameObject{1}, TestComponent{20, 2.0f});
    pool.Add(GameObject{2}, TestComponent{30, 3.0f});

    int32_t sum = 0;
    for (auto [entity, component] : pool)
        sum += component.value;

    EXPECT_EQ(sum, 60);
}

TEST_F(ComponentPoolTest, SizeTracksCorrectly)
{
    EXPECT_EQ(pool.GetSize(), 0);

    pool.Add(GameObject{0}, TestComponent{10, 1.0f});
    EXPECT_EQ(pool.GetSize(), 1);

    pool.Add(GameObject{1}, TestComponent{20, 2.0f});
    EXPECT_EQ(pool.GetSize(), 2);

    pool.Remove(GameObject{0});
    EXPECT_EQ(pool.GetSize(), 1);
}

TEST_F(ComponentPoolTest, ModifyThroughGetReference)
{
    pool.Add(GameObject{0}, TestComponent{10, 1.0f});

    auto retrieved = pool.Get(GameObject{0});
    ASSERT_TRUE(retrieved.has_value());
    retrieved->get().value = 99;

    auto again = pool.Get(GameObject{0});
    EXPECT_EQ(again->get().value, 99);
}

TEST_F(ComponentPoolTest, ReaddAfterRemove)
{
    pool.Add(GameObject{0}, TestComponent{10, 1.0f});
    pool.Remove(GameObject{0});
    pool.Add(GameObject{0}, TestComponent{99, 9.0f});

    EXPECT_TRUE(pool.Has(GameObject{0}));
    EXPECT_EQ(pool.Get(GameObject{0})->get().value, 99);
}

TEST_F(ComponentPoolTest, HasReturnsFalseForNonExistent)
{
    EXPECT_FALSE(pool.Has(GameObject{999}));
}

TEST_F(ComponentPoolTest, RemoveFromEmptyPoolIsSafe)
{
    pool.Remove(GameObject{0});
    EXPECT_EQ(pool.GetSize(), 0);
}
