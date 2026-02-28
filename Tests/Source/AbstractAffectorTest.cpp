#include <gtest/gtest.h>
#include "RenderStar/Common/Component/AbstractAffector.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/EntityAuthority.hpp"
#include "RenderStar/Common/Component/AuthorityContext.hpp"
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

    void StartWith(std::unique_ptr<ComponentModule> cm)
    {
        module = cm.get();
        manager = ModuleManager::Builder().Module(std::move(cm)).Build();
        manager->Start();
    }

    void TearDown() override
    {
        if (manager)
            manager->Shutdown();
    }
};

TEST_F(AbstractAffectorTest, AffectCalledWithCorrectModule)
{
    auto affector = std::make_unique<CountingAffector>();
    auto* ptr = affector.get();

    StartWith(ComponentModule::Builder().Affector(std::move(affector)).Build());

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

    StartWith(ComponentModule::Builder()
        .Affector(std::move(a1))
        .Affector(std::move(a2))
        .Build());

    module->RunAffectors();

    EXPECT_EQ(p1->myOrder, 0);
    EXPECT_EQ(p2->myOrder, 1);
}

TEST_F(AbstractAffectorTest, RunAffectorsWithNoAffectors)
{
    StartWith(std::make_unique<ComponentModule>());

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

    StartWith(ComponentModule::Builder()
        .Affector(std::make_unique<ModifyingAffector>())
        .Build());

    auto entity = module->CreateEntity();
    module->AddComponent<AffectorTestComp>(entity, AffectorTestComp{5});

    module->RunAffectors();

    EXPECT_EQ(module->GetComponent<AffectorTestComp>(entity)->get().value, 10);
}

TEST_F(AbstractAffectorTest, DefaultAuthorityContextIsNobody)
{
    auto affector = std::make_unique<CountingAffector>();
    auto* ptr = affector.get();

    StartWith(ComponentModule::Builder().Affector(std::move(affector)).Build());

    auto& ctx = ptr->GetAuthorityContext();
    EXPECT_EQ(ctx.level, AuthorityLevel::NOBODY);
    EXPECT_EQ(ctx.ownerId, -1);
}

TEST_F(AbstractAffectorTest, SetAuthorityContextAsServer)
{
    auto affector = std::make_unique<CountingAffector>();
    auto* ptr = affector.get();

    StartWith(ComponentModule::Builder().Affector(std::move(affector)).Build());

    ptr->SetAuthorityContext(AuthorityContext::AsServer());

    auto& ctx = ptr->GetAuthorityContext();
    EXPECT_EQ(ctx.level, AuthorityLevel::SERVER);
    EXPECT_EQ(ctx.ownerId, -1);
}

TEST_F(AbstractAffectorTest, SetAuthorityContextAsClient)
{
    auto affector = std::make_unique<CountingAffector>();
    auto* ptr = affector.get();

    StartWith(ComponentModule::Builder().Affector(std::move(affector)).Build());

    ptr->SetAuthorityContext(AuthorityContext::AsClient(5));

    auto& ctx = ptr->GetAuthorityContext();
    EXPECT_EQ(ctx.level, AuthorityLevel::CLIENT);
    EXPECT_EQ(ctx.ownerId, 5);
}

TEST_F(AbstractAffectorTest, AuthorityContextUsedInAffect)
{
    class AuthCheckingAffector final : public AbstractAffector
    {
    public:
        int modifiedCount = 0;

        void Affect(ComponentModule& cm) override
        {
            auto& pool = cm.GetPool<AffectorTestComp>();

            for (auto [entity, comp] : pool)
            {
                if (cm.CheckAuthority(entity, GetAuthorityContext()))
                    modifiedCount++;
            }
        }
    };

    auto affector = std::make_unique<AuthCheckingAffector>();
    auto* ptr = affector.get();

    StartWith(ComponentModule::Builder().Affector(std::move(affector)).Build());

    auto e1 = module->CreateEntity();
    module->AddComponent<AffectorTestComp>(e1, AffectorTestComp{1});
    module->SetEntityAuthority(e1, EntityAuthority::Client(1));

    auto e2 = module->CreateEntity();
    module->AddComponent<AffectorTestComp>(e2, AffectorTestComp{2});
    module->SetEntityAuthority(e2, EntityAuthority::Client(2));

    auto e3 = module->CreateEntity();
    module->AddComponent<AffectorTestComp>(e3, AffectorTestComp{3});
    module->SetEntityAuthority(e3, EntityAuthority::Server());

    ptr->SetAuthorityContext(AuthorityContext::AsClient(1));
    module->RunAffectors();

    EXPECT_EQ(ptr->modifiedCount, 1);
}

TEST_F(AbstractAffectorTest, OverwriteAuthorityContext)
{
    auto affector = std::make_unique<CountingAffector>();
    auto* ptr = affector.get();

    StartWith(ComponentModule::Builder().Affector(std::move(affector)).Build());

    ptr->SetAuthorityContext(AuthorityContext::AsServer());
    EXPECT_EQ(ptr->GetAuthorityContext().level, AuthorityLevel::SERVER);

    ptr->SetAuthorityContext(AuthorityContext::AsClient(10));
    EXPECT_EQ(ptr->GetAuthorityContext().level, AuthorityLevel::CLIENT);
    EXPECT_EQ(ptr->GetAuthorityContext().ownerId, 10);

    ptr->SetAuthorityContext(AuthorityContext::AsNobody());
    EXPECT_EQ(ptr->GetAuthorityContext().level, AuthorityLevel::NOBODY);
}
