#include <gtest/gtest.h>
#include "RenderStar/Common/Component/ComponentView.hpp"
#include "RenderStar/Common/Component/ComponentPool.hpp"
#include "RenderStar/Common/Component/GameObject.hpp"

using namespace RenderStar::Common::Component;

struct PositionComp { float x, y; };
struct VelocityComp { float vx, vy; };
struct HealthComp { int hp; };

class ComponentViewTest : public ::testing::Test
{
protected:
    ComponentPool<PositionComp> posPool;
    ComponentPool<VelocityComp> velPool;
    ComponentPool<HealthComp> hpPool;
};

TEST_F(ComponentViewTest, SinglePoolIteratesAll)
{
    posPool.Add(GameObject{0}, {1.0f, 2.0f});
    posPool.Add(GameObject{1}, {3.0f, 4.0f});
    posPool.Add(GameObject{2}, {5.0f, 6.0f});

    ComponentView view({&posPool});
    EXPECT_EQ(view.Count(), 3);
}

TEST_F(ComponentViewTest, TwoPoolIntersection)
{
    posPool.Add(GameObject{0}, {1.0f, 2.0f});
    posPool.Add(GameObject{1}, {3.0f, 4.0f});
    posPool.Add(GameObject{2}, {5.0f, 6.0f});

    velPool.Add(GameObject{1}, {0.1f, 0.2f});
    velPool.Add(GameObject{2}, {0.3f, 0.4f});

    ComponentView view({&posPool, &velPool});
    EXPECT_EQ(view.Count(), 2);
}

TEST_F(ComponentViewTest, ThreePoolIntersection)
{
    posPool.Add(GameObject{0}, {1.0f, 2.0f});
    posPool.Add(GameObject{1}, {3.0f, 4.0f});
    posPool.Add(GameObject{2}, {5.0f, 6.0f});

    velPool.Add(GameObject{0}, {0.1f, 0.2f});
    velPool.Add(GameObject{1}, {0.3f, 0.4f});
    velPool.Add(GameObject{2}, {0.5f, 0.6f});

    hpPool.Add(GameObject{1}, {100});

    ComponentView view({&posPool, &velPool, &hpPool});
    EXPECT_EQ(view.Count(), 1);
}

TEST_F(ComponentViewTest, EmptyPoolYieldsEmptyView)
{
    posPool.Add(GameObject{0}, {1.0f, 2.0f});

    ComponentView view({&posPool, &velPool});
    EXPECT_EQ(view.Count(), 0);
    EXPECT_TRUE(view.IsEmpty());
}

TEST_F(ComponentViewTest, CountMatchesIteration)
{
    posPool.Add(GameObject{0}, {1.0f, 2.0f});
    posPool.Add(GameObject{1}, {3.0f, 4.0f});

    velPool.Add(GameObject{0}, {0.1f, 0.2f});
    velPool.Add(GameObject{1}, {0.3f, 0.4f});

    ComponentView view({&posPool, &velPool});

    int32_t count = 0;
    for (auto entity : view)
    {
        (void)entity;
        ++count;
    }
    EXPECT_EQ(count, view.Count());
}

TEST_F(ComponentViewTest, IsEmptyOnNonEmptyView)
{
    posPool.Add(GameObject{0}, {1.0f, 2.0f});
    ComponentView view({&posPool});
    EXPECT_FALSE(view.IsEmpty());
}

TEST_F(ComponentViewTest, DefaultConstructedIsEmpty)
{
    ComponentView view;
    EXPECT_TRUE(view.IsEmpty());
    EXPECT_EQ(view.Count(), 0);
}

TEST_F(ComponentViewTest, RemovalFromPoolUpdatesView)
{
    posPool.Add(GameObject{0}, {1.0f, 2.0f});
    posPool.Add(GameObject{1}, {3.0f, 4.0f});

    velPool.Add(GameObject{0}, {0.1f, 0.2f});
    velPool.Add(GameObject{1}, {0.3f, 0.4f});

    velPool.Remove(GameObject{0});

    ComponentView view({&posPool, &velPool});
    EXPECT_EQ(view.Count(), 1);
}

TEST_F(ComponentViewTest, AllHaveChecksAllPools)
{
    posPool.Add(GameObject{0}, {1.0f, 2.0f});
    velPool.Add(GameObject{0}, {0.1f, 0.2f});

    ComponentView view({&posPool, &velPool});

    EXPECT_TRUE(view.AllHave(GameObject{0}));
    EXPECT_FALSE(view.AllHave(GameObject{1}));
}

TEST_F(ComponentViewTest, IterationYieldsCorrectEntities)
{
    posPool.Add(GameObject{0}, {1.0f, 2.0f});
    posPool.Add(GameObject{1}, {3.0f, 4.0f});
    posPool.Add(GameObject{2}, {5.0f, 6.0f});

    velPool.Add(GameObject{1}, {0.1f, 0.2f});

    ComponentView view({&posPool, &velPool});

    std::vector<int32_t> ids;
    for (auto entity : view)
        ids.push_back(entity.id);

    ASSERT_EQ(ids.size(), 1);
    EXPECT_EQ(ids[0], 1);
}
