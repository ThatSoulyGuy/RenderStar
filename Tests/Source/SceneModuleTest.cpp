#include <gtest/gtest.h>
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include "RenderStar/Common/Scene/SerializableComponent.hpp"
#include "RenderStar/Common/Scene/EntityIdRemapper.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Component/Components/Hierarchy.hpp"
#include "RenderStar/Common/Component/EntityAuthority.hpp"
#include "RenderStar/Common/Module/ModuleManager.hpp"
#include <filesystem>
#include <fstream>

using namespace RenderStar::Common::Scene;
using namespace RenderStar::Common::Component;
using namespace RenderStar::Common::Module;

template<>
struct RenderStar::Common::Scene::SerializableComponent<Transform> : std::true_type
{
    static constexpr const char* XmlTag = "Transform";

    static void Write(const Transform& t, pugi::xml_node& node)
    {
        node.append_attribute("px").set_value(t.position.x);
        node.append_attribute("py").set_value(t.position.y);
        node.append_attribute("pz").set_value(t.position.z);
        node.append_attribute("sx").set_value(t.scale.x);
        node.append_attribute("sy").set_value(t.scale.y);
        node.append_attribute("sz").set_value(t.scale.z);
    }

    static Transform Read(const pugi::xml_node& node)
    {
        Transform t;
        t.position.x = node.attribute("px").as_float();
        t.position.y = node.attribute("py").as_float();
        t.position.z = node.attribute("pz").as_float();
        t.scale.x = node.attribute("sx").as_float(1.0f);
        t.scale.y = node.attribute("sy").as_float(1.0f);
        t.scale.z = node.attribute("sz").as_float(1.0f);
        return t;
    }
};

class SceneModuleTest : public ::testing::Test
{
protected:
    std::unique_ptr<ModuleManager> manager;
    ComponentModule* ecs = nullptr;
    SceneModule* scene = nullptr;
    std::string tempFilePath;

    void SetUp() override
    {
        auto builder = ModuleManager::Builder();
        auto cm = std::make_unique<ComponentModule>();
        auto sm = std::make_unique<SceneModule>();
        ecs = cm.get();
        scene = sm.get();
        manager = builder.Module(std::move(cm)).Module(std::move(sm)).Build();
        manager->Start();

        scene->RegisterSerializableComponent<Transform>();

        tempFilePath = (std::filesystem::temp_directory_path() / "test_scene.xml").string();
    }

    void TearDown() override
    {
        manager->Shutdown();
        if (std::filesystem::exists(tempFilePath))
            std::filesystem::remove(tempFilePath);
    }
};

TEST_F(SceneModuleTest, CreateOwnedEntity)
{
    auto entity = scene->CreateEntity();
    EXPECT_TRUE(entity.IsValid());
    EXPECT_TRUE(scene->IsOwnedEntity(entity));
}

TEST_F(SceneModuleTest, CreateNamedOwnedEntity)
{
    auto entity = scene->CreateEntity("TestEntity");
    EXPECT_TRUE(scene->IsOwnedEntity(entity));
    auto name = ecs->GetEntityName(entity);
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(name->get(), "TestEntity");
}

TEST_F(SceneModuleTest, DestroyOwnedEntity)
{
    auto entity = scene->CreateEntity();
    scene->DestroyEntity(entity);
    EXPECT_FALSE(scene->IsOwnedEntity(entity));
    EXPECT_FALSE(ecs->EntityExists(entity));
}

TEST_F(SceneModuleTest, NoActiveSceneInitially)
{
    EXPECT_FALSE(scene->HasActiveScene());
}

TEST_F(SceneModuleTest, SaveAndLoad)
{
    auto entity = scene->CreateEntity("SaveTest");
    ecs->AddComponent<Transform>(entity, Transform{{1.0f, 2.0f, 3.0f}});

    SceneDescriptor descriptor;
    descriptor.name = "TestScene";
    descriptor.version = "1.0";
    descriptor.author = "Test";

    EXPECT_TRUE(scene->SaveScene(tempFilePath, descriptor));
    EXPECT_TRUE(scene->HasActiveScene());

    scene->ClearScene();
    EXPECT_FALSE(scene->HasActiveScene());

    EXPECT_TRUE(scene->LoadScene(tempFilePath));
    EXPECT_TRUE(scene->HasActiveScene());
}

TEST_F(SceneModuleTest, SceneDescriptorMetadata)
{
    auto entity = scene->CreateEntity();

    SceneDescriptor descriptor;
    descriptor.name = "MyScene";
    descriptor.version = "2.0";
    descriptor.author = "Author";
    descriptor.description = "A test scene";

    scene->SaveScene(tempFilePath, descriptor);

    auto& loaded = scene->GetCurrentSceneDescriptor();
    EXPECT_EQ(loaded.name, "MyScene");
}

TEST_F(SceneModuleTest, EmptySceneSaveLoad)
{
    SceneDescriptor descriptor;
    descriptor.name = "EmptyScene";

    EXPECT_TRUE(scene->SaveScene(tempFilePath, descriptor));

    scene->ClearScene();
    EXPECT_TRUE(scene->LoadScene(tempFilePath));
}

TEST_F(SceneModuleTest, ClearScene)
{
    auto entity = scene->CreateEntity();
    scene->ClearScene();
    EXPECT_FALSE(scene->IsOwnedEntity(entity));
    EXPECT_FALSE(scene->HasActiveScene());
}

TEST_F(SceneModuleTest, NonOwnedEntityNotTracked)
{
    auto entity = ecs->CreateEntity();
    EXPECT_FALSE(scene->IsOwnedEntity(entity));
}

TEST_F(SceneModuleTest, GetOwnedEntityIds)
{
    auto e1 = scene->CreateEntity("A");
    auto e2 = scene->CreateEntity("B");
    auto e3 = scene->CreateEntity("C");

    auto ids = scene->GetOwnedEntityIds();
    EXPECT_EQ(ids.size(), 3u);

    std::sort(ids.begin(), ids.end());
    std::vector<int32_t> expected = { e1.id, e2.id, e3.id };
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(ids, expected);
}

TEST_F(SceneModuleTest, GetOwnedEntityIdsAfterDestroy)
{
    auto e1 = scene->CreateEntity();
    auto e2 = scene->CreateEntity();
    scene->DestroyEntity(e1);

    auto ids = scene->GetOwnedEntityIds();
    ASSERT_EQ(ids.size(), 1u);
    EXPECT_EQ(ids[0], e2.id);
}

TEST_F(SceneModuleTest, SerializeEntities)
{
    auto e1 = scene->CreateEntity("Foo");
    ecs->AddComponent<Transform>(e1, Transform{{1.0f, 2.0f, 3.0f}});

    auto e2 = scene->CreateEntity("Bar");
    ecs->AddComponent<Transform>(e2, Transform{{4.0f, 5.0f, 6.0f}});

    auto xml = scene->SerializeEntities({ e1.id, e2.id });
    EXPECT_FALSE(xml.empty());
    EXPECT_NE(xml.find("Foo"), std::string::npos);
    EXPECT_NE(xml.find("Bar"), std::string::npos);
    EXPECT_NE(xml.find("Transform"), std::string::npos);
}

TEST_F(SceneModuleTest, SerializeDeserializeEntities)
{
    auto e1 = scene->CreateEntity("Alpha");
    auto& t1 = ecs->AddComponent<Transform>(e1);
    t1.position = glm::vec3(10.0f, 20.0f, 30.0f);
    t1.scale = glm::vec3(2.0f, 3.0f, 4.0f);

    auto e2 = scene->CreateEntity("Beta");
    auto& t2 = ecs->AddComponent<Transform>(e2);
    t2.position = glm::vec3(7.0f, 8.0f, 9.0f);

    auto xml = scene->SerializeEntities({ e1.id, e2.id });

    scene->ClearScene();
    EXPECT_EQ(scene->GetOwnedEntityIds().size(), 0u);

    EntityIdRemapper remapper;
    scene->DeserializeEntities(xml, remapper);
    scene->RemapEntityReferences(remapper);

    auto newIds = scene->GetOwnedEntityIds();
    ASSERT_EQ(newIds.size(), 2u);

    auto newE1 = remapper.Remap(e1.id);
    auto newE2 = remapper.Remap(e2.id);
    EXPECT_TRUE(newE1.IsValid());
    EXPECT_TRUE(newE2.IsValid());

    auto t1Opt = ecs->GetComponent<Transform>(newE1);
    ASSERT_TRUE(t1Opt.has_value());
    EXPECT_FLOAT_EQ(t1Opt->get().position.x, 10.0f);
    EXPECT_FLOAT_EQ(t1Opt->get().position.y, 20.0f);
    EXPECT_FLOAT_EQ(t1Opt->get().position.z, 30.0f);
    EXPECT_FLOAT_EQ(t1Opt->get().scale.x, 2.0f);
    EXPECT_FLOAT_EQ(t1Opt->get().scale.y, 3.0f);
    EXPECT_FLOAT_EQ(t1Opt->get().scale.z, 4.0f);

    auto t2Opt = ecs->GetComponent<Transform>(newE2);
    ASSERT_TRUE(t2Opt.has_value());
    EXPECT_FLOAT_EQ(t2Opt->get().position.x, 7.0f);
    EXPECT_FLOAT_EQ(t2Opt->get().position.y, 8.0f);
    EXPECT_FLOAT_EQ(t2Opt->get().position.z, 9.0f);
}

TEST_F(SceneModuleTest, DeserializeBatchesWithRemapper)
{
    auto e1 = scene->CreateEntity("Batch1Entity");
    auto& t1 = ecs->AddComponent<Transform>(e1);
    t1.position = glm::vec3(1.0f, 0.0f, 0.0f);

    auto e2 = scene->CreateEntity("Batch2Entity");
    auto& t2 = ecs->AddComponent<Transform>(e2);
    t2.position = glm::vec3(0.0f, 1.0f, 0.0f);

    auto xml1 = scene->SerializeEntities({ e1.id });
    auto xml2 = scene->SerializeEntities({ e2.id });

    scene->ClearScene();

    EntityIdRemapper remapper;
    scene->DeserializeEntities(xml1, remapper);
    scene->DeserializeEntities(xml2, remapper);
    scene->RemapEntityReferences(remapper);

    EXPECT_EQ(scene->GetOwnedEntityIds().size(), 2u);

    auto newE1 = remapper.Remap(e1.id);
    auto newE2 = remapper.Remap(e2.id);
    EXPECT_TRUE(newE1.IsValid());
    EXPECT_TRUE(newE2.IsValid());
    EXPECT_NE(newE1.id, newE2.id);

    auto t1Opt = ecs->GetComponent<Transform>(newE1);
    ASSERT_TRUE(t1Opt.has_value());
    EXPECT_FLOAT_EQ(t1Opt->get().position.x, 1.0f);

    auto t2Opt = ecs->GetComponent<Transform>(newE2);
    ASSERT_TRUE(t2Opt.has_value());
    EXPECT_FLOAT_EQ(t2Opt->get().position.y, 1.0f);
}

TEST_F(SceneModuleTest, DeserializePreservesEntityNames)
{
    auto entity = scene->CreateEntity("NamedEntity");
    ecs->AddComponent<Transform>(entity);

    auto xml = scene->SerializeEntities({ entity.id });
    scene->ClearScene();

    EntityIdRemapper remapper;
    scene->DeserializeEntities(xml, remapper);

    auto newEntity = remapper.Remap(entity.id);
    auto nameOpt = ecs->GetEntityName(newEntity);
    ASSERT_TRUE(nameOpt.has_value());
    EXPECT_EQ(nameOpt->get(), "NamedEntity");
}

TEST_F(SceneModuleTest, SerializeEmptyEntityList)
{
    auto xml = scene->SerializeEntities({});
    EXPECT_FALSE(xml.empty());

    EntityIdRemapper remapper;
    scene->DeserializeEntities(xml, remapper);
    EXPECT_EQ(remapper.GetAllMappings().size(), 0u);
}

TEST_F(SceneModuleTest, SerializeIncludesAuthorityNobody)
{
    auto entity = scene->CreateEntity("NobodyEntity");
    ecs->AddComponent<Transform>(entity);

    auto xml = scene->SerializeEntities({ entity.id });
    EXPECT_NE(xml.find("authority=\"nobody\""), std::string::npos);
}

TEST_F(SceneModuleTest, SerializeIncludesAuthorityServer)
{
    auto entity = scene->CreateEntity("ServerEntity");
    ecs->AddComponent<Transform>(entity);
    ecs->SetEntityAuthority(entity, EntityAuthority::Server());

    auto xml = scene->SerializeEntities({ entity.id });
    EXPECT_NE(xml.find("authority=\"server\""), std::string::npos);
}

TEST_F(SceneModuleTest, SerializeIncludesAuthorityClient)
{
    auto entity = scene->CreateEntity("ClientEntity");
    ecs->AddComponent<Transform>(entity);
    ecs->SetEntityAuthority(entity, EntityAuthority::Client(3));

    auto xml = scene->SerializeEntities({ entity.id });
    EXPECT_NE(xml.find("authority=\"client\""), std::string::npos);
    EXPECT_NE(xml.find("ownerId=\"3\""), std::string::npos);
}

TEST_F(SceneModuleTest, DeserializeRestoresAuthority)
{
    auto entity = scene->CreateEntity("AuthEntity");
    ecs->AddComponent<Transform>(entity);
    ecs->SetEntityAuthority(entity, EntityAuthority::Client(7));

    auto xml = scene->SerializeEntities({ entity.id });
    scene->ClearScene();

    EntityIdRemapper remapper;
    scene->DeserializeEntities(xml, remapper);

    auto newEntity = remapper.Remap(entity.id);
    ASSERT_TRUE(newEntity.IsValid());

    auto auth = ecs->GetEntityAuthority(newEntity);
    EXPECT_EQ(auth.level, AuthorityLevel::CLIENT);
    EXPECT_EQ(auth.ownerId, 7);
}

TEST_F(SceneModuleTest, DeserializeRestoresServerAuthority)
{
    auto entity = scene->CreateEntity("ServerAuth");
    ecs->AddComponent<Transform>(entity);
    ecs->SetEntityAuthority(entity, EntityAuthority::Server());

    auto xml = scene->SerializeEntities({ entity.id });
    scene->ClearScene();

    EntityIdRemapper remapper;
    scene->DeserializeEntities(xml, remapper);

    auto newEntity = remapper.Remap(entity.id);
    auto auth = ecs->GetEntityAuthority(newEntity);
    EXPECT_EQ(auth.level, AuthorityLevel::SERVER);
}

TEST_F(SceneModuleTest, DeserializeSkipsDuplicateServerIds)
{
    auto entity = scene->CreateEntity("Original");
    ecs->AddComponent<Transform>(entity, Transform{{1.0f, 2.0f, 3.0f}});

    auto xml = scene->SerializeEntities({ entity.id });
    scene->ClearScene();

    EntityIdRemapper remapper;
    scene->DeserializeEntities(xml, remapper);
    auto firstLocal = remapper.Remap(entity.id);

    scene->DeserializeEntities(xml, remapper);
    auto secondLocal = remapper.Remap(entity.id);

    EXPECT_EQ(firstLocal.id, secondLocal.id);
    EXPECT_EQ(scene->GetOwnedEntityIds().size(), 1u);
}

TEST_F(SceneModuleTest, DeserializeBatchThenCreateNoDuplicate)
{
    auto e1 = scene->CreateEntity("MapRoot");
    ecs->AddComponent<Transform>(e1);
    ecs->SetEntityAuthority(e1, EntityAuthority::Server());

    auto e2 = scene->CreateEntity("Player_0");
    ecs->AddComponent<Transform>(e2, Transform{{0.0f, 2.0f, 5.0f}});
    ecs->SetEntityAuthority(e2, EntityAuthority::Client(0));

    auto batchXml = scene->SerializeEntities({ e1.id, e2.id });
    auto createXml = scene->SerializeEntities({ e2.id });
    scene->ClearScene();

    EntityIdRemapper remapper;
    scene->DeserializeEntities(batchXml, remapper);
    EXPECT_EQ(scene->GetOwnedEntityIds().size(), 2u);

    scene->DeserializeEntities(createXml, remapper);
    EXPECT_EQ(scene->GetOwnedEntityIds().size(), 2u);
}

TEST_F(SceneModuleTest, UpdateEntityComponents)
{
    auto entity = scene->CreateEntity("Target");
    auto& t = ecs->AddComponent<Transform>(entity);
    t.position = glm::vec3(1.0f, 2.0f, 3.0f);

    auto updateXml = scene->SerializeEntities({ entity.id });

    t.position = glm::vec3(0.0f, 0.0f, 0.0f);
    EXPECT_FLOAT_EQ(t.position.x, 0.0f);

    scene->UpdateEntityComponents(entity, updateXml);

    auto updated = ecs->GetComponent<Transform>(entity);
    ASSERT_TRUE(updated.has_value());
    EXPECT_FLOAT_EQ(updated->get().position.x, 1.0f);
    EXPECT_FLOAT_EQ(updated->get().position.y, 2.0f);
    EXPECT_FLOAT_EQ(updated->get().position.z, 3.0f);
}

TEST_F(SceneModuleTest, UpdateEntityComponentsDoesNotCreateEntity)
{
    auto entity = scene->CreateEntity("Existing");
    ecs->AddComponent<Transform>(entity, Transform{{5.0f, 6.0f, 7.0f}});

    auto xml = scene->SerializeEntities({ entity.id });
    auto countBefore = scene->GetOwnedEntityIds().size();

    scene->UpdateEntityComponents(entity, xml);

    EXPECT_EQ(scene->GetOwnedEntityIds().size(), countBefore);
}

TEST_F(SceneModuleTest, UpdateEntityComponentsInvalidXml)
{
    auto entity = scene->CreateEntity("Entity");
    ecs->AddComponent<Transform>(entity, Transform{{1.0f, 1.0f, 1.0f}});

    scene->UpdateEntityComponents(entity, "not valid xml!!!");

    auto t = ecs->GetComponent<Transform>(entity);
    ASSERT_TRUE(t.has_value());
    EXPECT_FLOAT_EQ(t->get().position.x, 1.0f);
}

TEST_F(SceneModuleTest, DeserializeMultipleAuthorities)
{
    auto e1 = scene->CreateEntity("Server");
    ecs->AddComponent<Transform>(e1);
    ecs->SetEntityAuthority(e1, EntityAuthority::Server());

    auto e2 = scene->CreateEntity("Client1");
    ecs->AddComponent<Transform>(e2);
    ecs->SetEntityAuthority(e2, EntityAuthority::Client(1));

    auto e3 = scene->CreateEntity("Nobody");
    ecs->AddComponent<Transform>(e3);

    auto xml = scene->SerializeEntities({ e1.id, e2.id, e3.id });
    scene->ClearScene();

    EntityIdRemapper remapper;
    scene->DeserializeEntities(xml, remapper);

    auto newE1 = remapper.Remap(e1.id);
    auto newE2 = remapper.Remap(e2.id);
    auto newE3 = remapper.Remap(e3.id);

    EXPECT_EQ(ecs->GetEntityAuthority(newE1).level, AuthorityLevel::SERVER);
    EXPECT_EQ(ecs->GetEntityAuthority(newE2).level, AuthorityLevel::CLIENT);
    EXPECT_EQ(ecs->GetEntityAuthority(newE2).ownerId, 1);
    EXPECT_EQ(ecs->GetEntityAuthority(newE3).level, AuthorityLevel::NOBODY);
}
