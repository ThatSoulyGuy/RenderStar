#include <gtest/gtest.h>
#include "RenderStar/Common/Component/AbstractAffector.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Module/ModuleManager.hpp"

using namespace RenderStar::Common::Component;
using namespace RenderStar::Common::Module;

struct AffectorTestComp { int value; };

class CountingAffector final : public AbstractAffector
{
public:
    int callCount = 0;
    ComponentModule* receivedModule = nullptr;

    void Affect(ComponentModule& cm) override
    {
        ++callCount;
        receivedModule = &cm;
    }
};

class OrderTrackingAffector final : public AbstractAffector
{
public:
    int* counter;
    int myOrder = -1;

    explicit OrderTrackingAffector(int* c) : counter(c) {}

    void Affect(ComponentModule&) override
    {
        myOrder = (*counter)++;
    }
};

class AbstractAffectorTest : public ::testing::Test
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

TEST_F(AbstractAffectorTest, AffectCalledWithCorrectModule)
{
    auto affector = std::make_unique<CountingAffector>();
    auto* ptr = affector.get();
    module->RegisterSubModule(std::move(affector));

    module->RunAffectors();

    EXPECT_EQ(ptr->callCount, 1);
    EXPECT_EQ(ptr->receivedModule, module);
}

TEST_F(AbstractAffectorTest, MultipleAffectorsRunInOrder)
{
    int counter = 0;

    auto a1 = std::make_unique<OrderTrackingAffector>(&counter);
    auto a2 = std::make_unique<OrderTrackingAffector>(&counter);
    auto* p1 = a1.get();
    auto* p2 = a2.get();

    module->RegisterSubModule(std::move(a1));
    module->RegisterSubModule(std::move(a2));

    module->RunAffectors();

    EXPECT_EQ(p1->myOrder, 0);
    EXPECT_EQ(p2->myOrder, 1);
}

TEST_F(AbstractAffectorTest, RunAffectorsWithNoAffectors)
{
    module->RunAffectors();
}

TEST_F(AbstractAffectorTest, AffectorCanModifyComponents)
{
    class ModifyingAffector final : public AbstractAffector
    {
    public:
        void Affect(ComponentModule& cm) override
        {
            auto& pool = cm.GetPool<AffectorTestComp>();
            for (auto [entity, comp] : pool)
                comp.value *= 2;
        }
    };

    auto entity = module->CreateEntity();
    module->AddComponent<AffectorTestComp>(entity, AffectorTestComp{5});
    module->RegisterSubModule(std::make_unique<ModifyingAffector>());

    module->RunAffectors();

    EXPECT_EQ(module->GetComponent<AffectorTestComp>(entity)->get().value, 10);
}
