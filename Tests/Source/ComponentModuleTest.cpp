#include <gtest/gtest.h>
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/AbstractAffector.hpp"
#include "RenderStar/Common/Module/ModuleManager.hpp"

using namespace RenderStar::Common::Component;
using namespace RenderStar::Common::Module;

struct TestComp { int value; };
struct OtherComp { float data; };

class TestAffector final : public AbstractAffector
{
public:
    int callCount = 0;
    void Affect(ComponentModule&) override { ++callCount; }
};

class ComponentModuleTest : public ::testing::Test
{
protected:
    std::unique_ptr<ModuleManager> manager;
    ComponentModule* module = nullptr;

    void SetUp() override
    {
        auto builder = ModuleManager::Builder();
        auto cm = std::make_unique<ComponentModule>();
        module = cm.get();
        manager = builder.Module(std::move(cm)).Build();
        manager->Start();
    }

    void TearDown() override
    {
        manager->Shutdown();
    }
};

TEST_F(ComponentModuleTest, CreateEntityReturnsValid)
{
    auto entity = module->CreateEntity();
    EXPECT_TRUE(entity.IsValid());
}

TEST_F(ComponentModuleTest, CreateEntityIncrementingIds)
{
    auto e1 = module->CreateEntity();
    auto e2 = module->CreateEntity();
    EXPECT_NE(e1.id, e2.id);
}

TEST_F(ComponentModuleTest, CreateNamedEntity)
{
    auto entity = module->CreateEntity("Player");
    auto name = module->GetEntityName(entity);
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(name->get(), "Player");
}

TEST_F(ComponentModuleTest, FindEntityByName)
{
    auto entity = module->CreateEntity("Camera");
    auto found = module->FindEntityByName("Camera");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id, entity.id);
}

TEST_F(ComponentModuleTest, FindEntityByNameNotFound)
{
    auto found = module->FindEntityByName("NonExistent");
    EXPECT_FALSE(found.has_value());
}

TEST_F(ComponentModuleTest, DestroyEntity)
{
    auto entity = module->CreateEntity();
    EXPECT_TRUE(module->EntityExists(entity));
    module->DestroyEntity(entity);
    EXPECT_FALSE(module->EntityExists(entity));
}

TEST_F(ComponentModuleTest, AddAndGetComponent)
{
    auto entity = module->CreateEntity();
    module->AddComponent<TestComp>(entity, TestComp{42});

    auto comp = module->GetComponent<TestComp>(entity);
    ASSERT_TRUE(comp.has_value());
    EXPECT_EQ(comp->get().value, 42);
}

TEST_F(ComponentModuleTest, HasComponent)
{
    auto entity = module->CreateEntity();
    EXPECT_FALSE(module->HasComponent<TestComp>(entity));

    module->AddComponent<TestComp>(entity, TestComp{1});
    EXPECT_TRUE(module->HasComponent<TestComp>(entity));
}

TEST_F(ComponentModuleTest, RemoveComponent)
{
    auto entity = module->CreateEntity();
    module->AddComponent<TestComp>(entity, TestComp{1});
    module->RemoveComponent<TestComp>(entity);
    EXPECT_FALSE(module->HasComponent<TestComp>(entity));
}

TEST_F(ComponentModuleTest, MultipleComponentTypes)
{
    auto entity = module->CreateEntity();
    module->AddComponent<TestComp>(entity, TestComp{10});
    module->AddComponent<OtherComp>(entity, OtherComp{3.14f});

    EXPECT_TRUE(module->HasComponent<TestComp>(entity));
    EXPECT_TRUE(module->HasComponent<OtherComp>(entity));
    EXPECT_EQ(module->GetComponent<TestComp>(entity)->get().value, 10);
    EXPECT_FLOAT_EQ(module->GetComponent<OtherComp>(entity)->get().data, 3.14f);
}

TEST_F(ComponentModuleTest, GetPool)
{
    auto entity = module->CreateEntity();
    module->AddComponent<TestComp>(entity, TestComp{42});

    auto& pool = module->GetPool<TestComp>();
    EXPECT_EQ(pool.GetSize(), 1);
}

TEST_F(ComponentModuleTest, DestroyEntityRemovesComponents)
{
    auto entity = module->CreateEntity();
    module->AddComponent<TestComp>(entity, TestComp{1});
    module->DestroyEntity(entity);

    EXPECT_FALSE(module->HasComponent<TestComp>(entity));
}

TEST_F(ComponentModuleTest, RunAffectorsCallsRegistered)
{
    auto affector = std::make_unique<TestAffector>();
    auto* affectorPtr = affector.get();
    module->RegisterSubModule(std::move(affector));

    module->RunAffectors();
    EXPECT_EQ(affectorPtr->callCount, 1);

    module->RunAffectors();
    EXPECT_EQ(affectorPtr->callCount, 2);
}

TEST_F(ComponentModuleTest, EntityExistsAfterCreation)
{
    auto entity = module->CreateEntity();
    EXPECT_TRUE(module->EntityExists(entity));
}

TEST_F(ComponentModuleTest, AddComponentDefaultConstruct)
{
    auto entity = module->CreateEntity();
    auto& comp = module->AddComponent<TestComp>(entity);
    comp.value = 77;

    EXPECT_EQ(module->GetComponent<TestComp>(entity)->get().value, 77);
}
