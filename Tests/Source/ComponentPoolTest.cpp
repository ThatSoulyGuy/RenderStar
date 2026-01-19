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

    void SetUp() override
    {
    }
};

TEST_F(ComponentPoolTest, AddComponentToEntity)
{
    GameObject entity{0};
    TestComponent component{42, 1.5f};

    pool.Add(entity, component);

    EXPECT_TRUE(pool.Has(entity));
}

TEST_F(ComponentPoolTest, GetComponentReturnsCorrectValue)
{
    GameObject entity{0};
    TestComponent component{42, 1.5f};

    pool.Add(entity, component);
    auto retrieved = pool.Get(entity);

    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->get().value, 42);
    EXPECT_FLOAT_EQ(retrieved->get().multiplier, 1.5f);
}

TEST_F(ComponentPoolTest, RemoveComponentFromEntity)
{
    GameObject entity{0};
    TestComponent component{42, 1.5f};

    pool.Add(entity, component);
    pool.Remove(entity);

    EXPECT_FALSE(pool.Has(entity));
}

TEST_F(ComponentPoolTest, GetNonExistentComponentReturnsEmpty)
{
    GameObject entity{0};
    auto retrieved = pool.Get(entity);

    EXPECT_FALSE(retrieved.has_value());
}

TEST_F(ComponentPoolTest, MultipleEntitiesHaveCorrectComponents)
{
    GameObject entity1{0};
    GameObject entity2{1};
    GameObject entity3{2};

    pool.Add(entity1, TestComponent{10, 1.0f});
    pool.Add(entity2, TestComponent{20, 2.0f});
    pool.Add(entity3, TestComponent{30, 3.0f});

    auto comp1 = pool.Get(entity1);
    auto comp2 = pool.Get(entity2);
    auto comp3 = pool.Get(entity3);

    ASSERT_TRUE(comp1.has_value());
    ASSERT_TRUE(comp2.has_value());
    ASSERT_TRUE(comp3.has_value());

    EXPECT_EQ(comp1->get().value, 10);
    EXPECT_EQ(comp2->get().value, 20);
    EXPECT_EQ(comp3->get().value, 30);
}

TEST_F(ComponentPoolTest, RemoveMiddleEntityMaintainsOthers)
{
    GameObject entity1{0};
    GameObject entity2{1};
    GameObject entity3{2};

    pool.Add(entity1, TestComponent{10, 1.0f});
    pool.Add(entity2, TestComponent{20, 2.0f});
    pool.Add(entity3, TestComponent{30, 3.0f});

    pool.Remove(entity2);

    EXPECT_TRUE(pool.Has(entity1));
    EXPECT_FALSE(pool.Has(entity2));
    EXPECT_TRUE(pool.Has(entity3));

    auto comp1 = pool.Get(entity1);
    auto comp3 = pool.Get(entity3);

    ASSERT_TRUE(comp1.has_value());
    ASSERT_TRUE(comp3.has_value());

    EXPECT_EQ(comp1->get().value, 10);
    EXPECT_EQ(comp3->get().value, 30);
}

TEST_F(ComponentPoolTest, IterateOverComponents)
{
    pool.Add(GameObject{0}, TestComponent{10, 1.0f});
    pool.Add(GameObject{1}, TestComponent{20, 2.0f});
    pool.Add(GameObject{2}, TestComponent{30, 3.0f});

    int32_t sum = 0;
    for (auto entry : pool)
    {
        sum += entry.component.value;
    }

    EXPECT_EQ(sum, 60);
}

TEST_F(ComponentPoolTest, SizeReturnsCorrectValue)
{
    EXPECT_EQ(pool.Size(), 0);

    pool.Add(GameObject{0}, TestComponent{10, 1.0f});
    EXPECT_EQ(pool.Size(), 1);

    pool.Add(GameObject{1}, TestComponent{20, 2.0f});
    EXPECT_EQ(pool.Size(), 2);

    pool.Remove(GameObject{0});
    EXPECT_EQ(pool.Size(), 1);
}
