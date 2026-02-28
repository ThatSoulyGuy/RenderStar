#include <gtest/gtest.h>
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/EntityAuthority.hpp"
#include "RenderStar/Common/Component/AuthorityContext.hpp"
#include "RenderStar/Common/Module/ModuleManager.hpp"

using namespace RenderStar::Common::Component;
using namespace RenderStar::Common::Module;

struct AuthTestComp { int value = 0; };
struct AuthOtherComp { float data = 0.0f; };

class ComponentModuleAuthorityTest : public ::testing::Test
{
protected:
    std::unique_ptr<ModuleManager> manager;
    ComponentModule* module = nullptr;

    void SetUp() override
    {
        auto cm = std::make_unique<ComponentModule>();
        module = cm.get();
        manager = ModuleManager::Builder().Module(std::move(cm)).Build();
        manager->Start();
    }

    void TearDown() override
    {
        manager->Shutdown();
    }
};

TEST_F(ComponentModuleAuthorityTest, DefaultAuthorityIsNobody)
{
    auto entity = module->CreateEntity();
    auto auth = module->GetEntityAuthority(entity);
    EXPECT_EQ(auth.level, AuthorityLevel::NOBODY);
    EXPECT_EQ(auth.ownerId, -1);
}

TEST_F(ComponentModuleAuthorityTest, SetAndGetServerAuthority)
{
    auto entity = module->CreateEntity();
    module->SetEntityAuthority(entity, EntityAuthority::Server());

    auto auth = module->GetEntityAuthority(entity);
    EXPECT_EQ(auth.level, AuthorityLevel::SERVER);
    EXPECT_EQ(auth.ownerId, -1);
}

TEST_F(ComponentModuleAuthorityTest, SetAndGetClientAuthority)
{
    auto entity = module->CreateEntity();
    module->SetEntityAuthority(entity, EntityAuthority::Client(3));

    auto auth = module->GetEntityAuthority(entity);
    EXPECT_EQ(auth.level, AuthorityLevel::CLIENT);
    EXPECT_EQ(auth.ownerId, 3);
}

TEST_F(ComponentModuleAuthorityTest, OverwriteAuthority)
{
    auto entity = module->CreateEntity();
    module->SetEntityAuthority(entity, EntityAuthority::Server());
    module->SetEntityAuthority(entity, EntityAuthority::Client(7));

    auto auth = module->GetEntityAuthority(entity);
    EXPECT_EQ(auth.level, AuthorityLevel::CLIENT);
    EXPECT_EQ(auth.ownerId, 7);
}

TEST_F(ComponentModuleAuthorityTest, DestroyEntityClearsAuthority)
{
    auto entity = module->CreateEntity();
    module->SetEntityAuthority(entity, EntityAuthority::Server());
    module->DestroyEntity(entity);

    auto auth = module->GetEntityAuthority(entity);
    EXPECT_EQ(auth.level, AuthorityLevel::NOBODY);
}

TEST_F(ComponentModuleAuthorityTest, CheckAuthorityServerEntity)
{
    auto entity = module->CreateEntity();
    module->SetEntityAuthority(entity, EntityAuthority::Server());

    EXPECT_TRUE(module->CheckAuthority(entity, AuthorityContext::AsServer()));
    EXPECT_FALSE(module->CheckAuthority(entity, AuthorityContext::AsClient(0)));
    EXPECT_FALSE(module->CheckAuthority(entity, AuthorityContext::AsNobody()));
}

TEST_F(ComponentModuleAuthorityTest, CheckAuthorityClientEntity)
{
    auto entity = module->CreateEntity();
    module->SetEntityAuthority(entity, EntityAuthority::Client(5));

    EXPECT_TRUE(module->CheckAuthority(entity, AuthorityContext::AsClient(5)));
    EXPECT_FALSE(module->CheckAuthority(entity, AuthorityContext::AsClient(6)));
    EXPECT_FALSE(module->CheckAuthority(entity, AuthorityContext::AsServer()));
    EXPECT_FALSE(module->CheckAuthority(entity, AuthorityContext::AsNobody()));
}

TEST_F(ComponentModuleAuthorityTest, CheckAuthorityNobodyEntity)
{
    auto entity = module->CreateEntity();

    EXPECT_TRUE(module->CheckAuthority(entity, AuthorityContext::AsServer()));
    EXPECT_TRUE(module->CheckAuthority(entity, AuthorityContext::AsClient(0)));
    EXPECT_TRUE(module->CheckAuthority(entity, AuthorityContext::AsNobody()));
}

TEST_F(ComponentModuleAuthorityTest, MultipleEntitiesDifferentAuthority)
{
    auto e1 = module->CreateEntity();
    auto e2 = module->CreateEntity();
    auto e3 = module->CreateEntity();

    module->SetEntityAuthority(e1, EntityAuthority::Server());
    module->SetEntityAuthority(e2, EntityAuthority::Client(1));
    module->SetEntityAuthority(e3, EntityAuthority::Nobody());

    EXPECT_TRUE(module->CheckAuthority(e1, AuthorityContext::AsServer()));
    EXPECT_FALSE(module->CheckAuthority(e1, AuthorityContext::AsClient(1)));

    EXPECT_TRUE(module->CheckAuthority(e2, AuthorityContext::AsClient(1)));
    EXPECT_FALSE(module->CheckAuthority(e2, AuthorityContext::AsServer()));

    EXPECT_TRUE(module->CheckAuthority(e3, AuthorityContext::AsServer()));
    EXPECT_TRUE(module->CheckAuthority(e3, AuthorityContext::AsClient(99)));
}

TEST_F(ComponentModuleAuthorityTest, GetComponentAuthorizedSucceeds)
{
    auto entity = module->CreateEntity();
    module->AddComponent<AuthTestComp>(entity, AuthTestComp{42});
    module->SetEntityAuthority(entity, EntityAuthority::Client(1));

    auto result = module->GetComponentAuthorized<AuthTestComp>(entity, AuthorityContext::AsClient(1));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->get().value, 42);
}

TEST_F(ComponentModuleAuthorityTest, GetComponentAuthorizedDenied)
{
    auto entity = module->CreateEntity();
    module->AddComponent<AuthTestComp>(entity, AuthTestComp{42});
    module->SetEntityAuthority(entity, EntityAuthority::Client(1));

    auto result = module->GetComponentAuthorized<AuthTestComp>(entity, AuthorityContext::AsClient(2));
    EXPECT_FALSE(result.has_value());
}

TEST_F(ComponentModuleAuthorityTest, GetComponentAuthorizedMarksDirty)
{
    auto entity = module->CreateEntity();
    module->AddComponent<AuthTestComp>(entity, AuthTestComp{10});
    module->SetEntityAuthority(entity, EntityAuthority::Client(1));

    module->ConsumeDirtyEntities();

    module->GetComponentAuthorized<AuthTestComp>(entity, AuthorityContext::AsClient(1));

    auto dirty = module->ConsumeDirtyEntities();
    EXPECT_TRUE(dirty.contains(entity.id));
}

TEST_F(ComponentModuleAuthorityTest, GetComponentAuthorizedDeniedDoesNotMarkDirty)
{
    auto entity = module->CreateEntity();
    module->AddComponent<AuthTestComp>(entity, AuthTestComp{10});
    module->SetEntityAuthority(entity, EntityAuthority::Client(1));

    module->ConsumeDirtyEntities();

    module->GetComponentAuthorized<AuthTestComp>(entity, AuthorityContext::AsClient(2));

    auto dirty = module->ConsumeDirtyEntities();
    EXPECT_FALSE(dirty.contains(entity.id));
}

TEST_F(ComponentModuleAuthorityTest, AddComponentAuthorizedSucceeds)
{
    auto entity = module->CreateEntity();
    module->SetEntityAuthority(entity, EntityAuthority::Client(1));

    auto& comp = module->AddComponentAuthorized<AuthTestComp>(entity, AuthorityContext::AsClient(1));
    comp.value = 99;

    EXPECT_TRUE(module->HasComponent<AuthTestComp>(entity));
    EXPECT_EQ(module->GetComponent<AuthTestComp>(entity)->get().value, 99);
}

TEST_F(ComponentModuleAuthorityTest, AddComponentAuthorizedDenied)
{
    auto entity = module->CreateEntity();
    module->SetEntityAuthority(entity, EntityAuthority::Server());

    module->AddComponentAuthorized<AuthTestComp>(entity, AuthorityContext::AsClient(0));

    EXPECT_FALSE(module->HasComponent<AuthTestComp>(entity));
}

TEST_F(ComponentModuleAuthorityTest, AddComponentWithValueAuthorizedSucceeds)
{
    auto entity = module->CreateEntity();
    module->SetEntityAuthority(entity, EntityAuthority::Nobody());

    module->AddComponentAuthorized<AuthTestComp>(entity, AuthTestComp{77}, AuthorityContext::AsClient(0));

    ASSERT_TRUE(module->HasComponent<AuthTestComp>(entity));
    EXPECT_EQ(module->GetComponent<AuthTestComp>(entity)->get().value, 77);
}

TEST_F(ComponentModuleAuthorityTest, AddComponentWithValueAuthorizedDenied)
{
    auto entity = module->CreateEntity();
    module->SetEntityAuthority(entity, EntityAuthority::Client(1));

    module->AddComponentAuthorized<AuthTestComp>(entity, AuthTestComp{77}, AuthorityContext::AsServer());

    EXPECT_FALSE(module->HasComponent<AuthTestComp>(entity));
}

TEST_F(ComponentModuleAuthorityTest, RemoveComponentAuthorizedSucceeds)
{
    auto entity = module->CreateEntity();
    module->AddComponent<AuthTestComp>(entity, AuthTestComp{1});
    module->SetEntityAuthority(entity, EntityAuthority::Server());

    module->RemoveComponentAuthorized<AuthTestComp>(entity, AuthorityContext::AsServer());

    EXPECT_FALSE(module->HasComponent<AuthTestComp>(entity));
}

TEST_F(ComponentModuleAuthorityTest, RemoveComponentAuthorizedDenied)
{
    auto entity = module->CreateEntity();
    module->AddComponent<AuthTestComp>(entity, AuthTestComp{1});
    module->SetEntityAuthority(entity, EntityAuthority::Server());

    module->RemoveComponentAuthorized<AuthTestComp>(entity, AuthorityContext::AsClient(0));

    EXPECT_TRUE(module->HasComponent<AuthTestComp>(entity));
}

TEST_F(ComponentModuleAuthorityTest, MarkEntityDirty)
{
    auto entity = module->CreateEntity();
    module->MarkEntityDirty(entity);

    auto dirty = module->ConsumeDirtyEntities();
    EXPECT_TRUE(dirty.contains(entity.id));
}

TEST_F(ComponentModuleAuthorityTest, ConsumeDirtyEntitiesClearsSet)
{
    auto entity = module->CreateEntity();
    module->MarkEntityDirty(entity);

    auto dirty1 = module->ConsumeDirtyEntities();
    EXPECT_EQ(dirty1.size(), 1u);

    auto dirty2 = module->ConsumeDirtyEntities();
    EXPECT_TRUE(dirty2.empty());
}

TEST_F(ComponentModuleAuthorityTest, MultipleDirtyEntities)
{
    auto e1 = module->CreateEntity();
    auto e2 = module->CreateEntity();
    auto e3 = module->CreateEntity();

    module->MarkEntityDirty(e1);
    module->MarkEntityDirty(e2);

    auto dirty = module->ConsumeDirtyEntities();
    EXPECT_EQ(dirty.size(), 2u);
    EXPECT_TRUE(dirty.contains(e1.id));
    EXPECT_TRUE(dirty.contains(e2.id));
    EXPECT_FALSE(dirty.contains(e3.id));
}

TEST_F(ComponentModuleAuthorityTest, MarkSameEntityDirtyTwice)
{
    auto entity = module->CreateEntity();
    module->MarkEntityDirty(entity);
    module->MarkEntityDirty(entity);

    auto dirty = module->ConsumeDirtyEntities();
    EXPECT_EQ(dirty.size(), 1u);
    EXPECT_TRUE(dirty.contains(entity.id));
}

TEST_F(ComponentModuleAuthorityTest, DestroyEntityClearsDirtyFlag)
{
    auto entity = module->CreateEntity();
    module->MarkEntityDirty(entity);
    module->DestroyEntity(entity);

    auto dirty = module->ConsumeDirtyEntities();
    EXPECT_FALSE(dirty.contains(entity.id));
}

TEST_F(ComponentModuleAuthorityTest, NoDirtyEntitiesInitially)
{
    auto dirty = module->ConsumeDirtyEntities();
    EXPECT_TRUE(dirty.empty());
}

TEST_F(ComponentModuleAuthorityTest, AuthorizedOperationsMarkDirty)
{
    auto entity = module->CreateEntity();
    module->SetEntityAuthority(entity, EntityAuthority::Nobody());

    module->AddComponentAuthorized<AuthTestComp>(entity, AuthorityContext::AsServer());
    auto dirty1 = module->ConsumeDirtyEntities();
    EXPECT_TRUE(dirty1.contains(entity.id));

    module->GetComponentAuthorized<AuthTestComp>(entity, AuthorityContext::AsServer());
    auto dirty2 = module->ConsumeDirtyEntities();
    EXPECT_TRUE(dirty2.contains(entity.id));

    module->RemoveComponentAuthorized<AuthTestComp>(entity, AuthorityContext::AsServer());
    auto dirty3 = module->ConsumeDirtyEntities();
    EXPECT_TRUE(dirty3.contains(entity.id));
}

TEST_F(ComponentModuleAuthorityTest, UngatedMethodsDoNotMarkDirty)
{
    auto entity = module->CreateEntity();
    module->AddComponent<AuthTestComp>(entity, AuthTestComp{1});
    module->GetComponent<AuthTestComp>(entity);
    module->RemoveComponent<AuthTestComp>(entity);

    auto dirty = module->ConsumeDirtyEntities();
    EXPECT_TRUE(dirty.empty());
}

TEST_F(ComponentModuleAuthorityTest, UngatedReadsAlwaysWork)
{
    auto entity = module->CreateEntity();
    module->AddComponent<AuthTestComp>(entity, AuthTestComp{42});
    module->SetEntityAuthority(entity, EntityAuthority::Server());

    auto result = module->GetComponent<AuthTestComp>(entity);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->get().value, 42);
}
