#include <gtest/gtest.h>
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include "RenderStar/Common/Scene/SceneDescriptor.hpp"
#include "RenderStar/Common/Scene/EntityIdRemapper.hpp"
#include "RenderStar/Common/Scene/ComponentSerializerRegistry.hpp"
#include "RenderStar/Common/Scene/SceneEvents.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Component/Components/Hierarchy.hpp"
#include "RenderStar/Common/Module/ModuleManager.hpp"
#include <pugixml.hpp>
#include <filesystem>
#include <fstream>

using namespace RenderStar::Common;
using namespace RenderStar::Common::Scene;
using namespace RenderStar::Common::Component;

// ============================================================
// Test component types
// ============================================================

struct Health
{
    float current{ 100.0f };
    float maximum{ 100.0f };
};

struct FollowTarget
{
    GameObject target{ GameObject::INVALID_ID };
    float distance{ 5.0f };
};

// ============================================================
// Test fixture: sets up ModuleManager with ComponentModule + SceneModule
// ============================================================

class SceneModuleTest : public ::testing::Test
{
protected:

    std::unique_ptr<Module::ModuleManager> moduleManager;
    SceneModule* sceneModule = nullptr;
    ComponentModule* componentModule = nullptr;
    std::string tempFilePath;

    void SetUp() override
    {
        moduleManager = Module::ModuleManager::Builder()
            .Module(std::make_unique<ComponentModule>())
            .Module(std::make_unique<SceneModule>())
            .Build();

        moduleManager->Start();

        auto& ctx = moduleManager->GetContext();
        sceneModule = &ctx.GetModule<SceneModule>().value().get();
        componentModule = &ctx.GetModule<ComponentModule>().value().get();

        tempFilePath = (std::filesystem::temp_directory_path() / "renderstar_test_scene.xml").string();

        RegisterTestSerializers();
    }

    void TearDown() override
    {
        moduleManager->Shutdown();
        moduleManager.reset();

        if (std::filesystem::exists(tempFilePath))
            std::filesystem::remove(tempFilePath);
    }

    void RegisterTestSerializers()
    {
        sceneModule->RegisterComponentSerializer<Transform>("Transform", {
            [](const Transform& t, pugi::xml_node& node)
            {
                auto pos = node.append_child("Position");
                pos.append_attribute("x").set_value(t.position.x);
                pos.append_attribute("y").set_value(t.position.y);
                pos.append_attribute("z").set_value(t.position.z);

                auto rot = node.append_child("Rotation");
                rot.append_attribute("w").set_value(t.rotation.w);
                rot.append_attribute("x").set_value(t.rotation.x);
                rot.append_attribute("y").set_value(t.rotation.y);
                rot.append_attribute("z").set_value(t.rotation.z);

                auto scl = node.append_child("Scale");
                scl.append_attribute("x").set_value(t.scale.x);
                scl.append_attribute("y").set_value(t.scale.y);
                scl.append_attribute("z").set_value(t.scale.z);
            },
            [](const pugi::xml_node& node) -> Transform
            {
                Transform t{};

                if (auto pos = node.child("Position"); !pos.empty())
                {
                    t.position.x = pos.attribute("x").as_float(0.0f);
                    t.position.y = pos.attribute("y").as_float(0.0f);
                    t.position.z = pos.attribute("z").as_float(0.0f);
                }

                if (auto rot = node.child("Rotation"); !rot.empty())
                {
                    t.rotation.w = rot.attribute("w").as_float(1.0f);
                    t.rotation.x = rot.attribute("x").as_float(0.0f);
                    t.rotation.y = rot.attribute("y").as_float(0.0f);
                    t.rotation.z = rot.attribute("z").as_float(0.0f);
                }

                if (auto scl = node.child("Scale"); !scl.empty())
                {
                    t.scale.x = scl.attribute("x").as_float(1.0f);
                    t.scale.y = scl.attribute("y").as_float(1.0f);
                    t.scale.z = scl.attribute("z").as_float(1.0f);
                }

                return t;
            },
            nullptr
        });

        sceneModule->RegisterComponentSerializer<Hierarchy>("Hierarchy", {
            [](const Hierarchy& h, pugi::xml_node& node)
            {
                if (h.HasParent())
                    node.append_attribute("parent").set_value(h.parent.id);

                for (const auto& child : h.children)
                    node.append_child("Child").append_attribute("id").set_value(child.id);
            },
            [](const pugi::xml_node& node) -> Hierarchy
            {
                Hierarchy h{};

                if (auto parentAttr = node.attribute("parent"); !parentAttr.empty())
                    h.parent = GameObject{ parentAttr.as_int(GameObject::INVALID_ID) };

                for (auto childNode = node.child("Child"); childNode; childNode = childNode.next_sibling("Child"))
                    h.children.push_back(GameObject{ childNode.attribute("id").as_int(GameObject::INVALID_ID) });

                return h;
            },
            [](Hierarchy& h, const EntityIdRemapper& remapper)
            {
                if (h.HasParent())
                    h.parent = remapper.Remap(h.parent.id);

                for (auto& child : h.children)
                    child = remapper.Remap(child.id);
            }
        });

        sceneModule->RegisterComponentSerializer<Health>("Health", {
            [](const Health& h, pugi::xml_node& node)
            {
                node.append_attribute("current").set_value(h.current);
                node.append_attribute("maximum").set_value(h.maximum);
            },
            [](const pugi::xml_node& node) -> Health
            {
                return Health{
                    node.attribute("current").as_float(100.0f),
                    node.attribute("maximum").as_float(100.0f)
                };
            },
            nullptr
        });

        sceneModule->RegisterComponentSerializer<FollowTarget>("FollowTarget", {
            [](const FollowTarget& ft, pugi::xml_node& node)
            {
                node.append_attribute("target").set_value(ft.target.id);
                node.append_attribute("distance").set_value(ft.distance);
            },
            [](const pugi::xml_node& node) -> FollowTarget
            {
                return FollowTarget{
                    GameObject{ node.attribute("target").as_int(GameObject::INVALID_ID) },
                    node.attribute("distance").as_float(5.0f)
                };
            },
            [](FollowTarget& ft, const EntityIdRemapper& remapper)
            {
                if (ft.target.IsValid())
                    ft.target = remapper.Remap(ft.target.id);
            }
        });
    }
};

// ============================================================
// EntityIdRemapper tests
// ============================================================

TEST(EntityIdRemapperTest, RecordAndRemap)
{
    EntityIdRemapper remapper;

    remapper.RecordMapping(0, GameObject{ 10 });
    remapper.RecordMapping(1, GameObject{ 11 });

    EXPECT_EQ(remapper.Remap(0).id, 10);
    EXPECT_EQ(remapper.Remap(1).id, 11);
}

TEST(EntityIdRemapperTest, RemapUnknownReturnsInvalid)
{
    EntityIdRemapper remapper;

    auto result = remapper.Remap(99);
    EXPECT_EQ(result.id, GameObject::INVALID_ID);
}

TEST(EntityIdRemapperTest, HasMapping)
{
    EntityIdRemapper remapper;

    remapper.RecordMapping(5, GameObject{ 42 });

    EXPECT_TRUE(remapper.HasMapping(5));
    EXPECT_FALSE(remapper.HasMapping(6));
}

// ============================================================
// Entity ownership tests
// ============================================================

TEST_F(SceneModuleTest, CreateEntityIsOwned)
{
    auto entity = sceneModule->CreateEntity();

    EXPECT_TRUE(entity.IsValid());
    EXPECT_TRUE(sceneModule->IsOwnedEntity(entity));
    EXPECT_TRUE(componentModule->EntityExists(entity));
}

TEST_F(SceneModuleTest, CreateNamedEntityIsOwned)
{
    auto entity = sceneModule->CreateEntity("TestEntity");

    EXPECT_TRUE(entity.IsValid());
    EXPECT_TRUE(sceneModule->IsOwnedEntity(entity));

    auto name = componentModule->GetEntityName(entity);
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(name.value().get(), "TestEntity");
}

TEST_F(SceneModuleTest, DirectlyCreatedEntityIsNotOwned)
{
    auto entity = componentModule->CreateEntity();

    EXPECT_TRUE(entity.IsValid());
    EXPECT_FALSE(sceneModule->IsOwnedEntity(entity));
}

TEST_F(SceneModuleTest, DestroyEntityRemovesOwnership)
{
    auto entity = sceneModule->CreateEntity();
    EXPECT_TRUE(sceneModule->IsOwnedEntity(entity));

    sceneModule->DestroyEntity(entity);
    EXPECT_FALSE(sceneModule->IsOwnedEntity(entity));
    EXPECT_FALSE(componentModule->EntityExists(entity));
}

// ============================================================
// Save and load tests
// ============================================================

TEST_F(SceneModuleTest, SaveAndLoadEmptyScene)
{
    SceneDescriptor desc;
    desc.name = "EmptyScene";
    desc.version = "1.0";
    desc.author = "Test";
    desc.description = "An empty test scene";

    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));
    EXPECT_TRUE(sceneModule->HasActiveScene());

    sceneModule->ClearScene();
    EXPECT_FALSE(sceneModule->HasActiveScene());

    EXPECT_TRUE(sceneModule->LoadScene(tempFilePath));
    EXPECT_TRUE(sceneModule->HasActiveScene());

    const auto& loaded = sceneModule->GetCurrentSceneDescriptor();
    EXPECT_EQ(loaded.name, "EmptyScene");
    EXPECT_EQ(loaded.version, "1.0");
    EXPECT_EQ(loaded.author, "Test");
    EXPECT_EQ(loaded.description, "An empty test scene");
}

TEST_F(SceneModuleTest, SaveAndLoadTransformComponent)
{
    auto entity = sceneModule->CreateEntity("Cube");

    Transform t{};
    t.position = glm::vec3(1.0f, 2.0f, 3.0f);
    t.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    t.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    componentModule->AddComponent<Transform>(entity, t);

    SceneDescriptor desc;
    desc.name = "TransformTest";
    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));

    sceneModule->ClearScene();
    EXPECT_TRUE(sceneModule->LoadScene(tempFilePath));

    auto loadedEntity = componentModule->FindEntityByName("Cube");
    ASSERT_TRUE(loadedEntity.has_value());

    auto transform = componentModule->GetComponent<Transform>(loadedEntity.value());
    ASSERT_TRUE(transform.has_value());

    const auto& lt = transform.value().get();
    EXPECT_FLOAT_EQ(lt.position.x, 1.0f);
    EXPECT_FLOAT_EQ(lt.position.y, 2.0f);
    EXPECT_FLOAT_EQ(lt.position.z, 3.0f);
    EXPECT_FLOAT_EQ(lt.scale.x, 0.5f);
    EXPECT_FLOAT_EQ(lt.scale.y, 0.5f);
    EXPECT_FLOAT_EQ(lt.scale.z, 0.5f);
}

TEST_F(SceneModuleTest, SaveAndLoadMultipleEntities)
{
    auto e1 = sceneModule->CreateEntity("Entity1");
    auto e2 = sceneModule->CreateEntity("Entity2");
    auto e3 = sceneModule->CreateEntity("Entity3");

    componentModule->AddComponent<Health>(e1, Health{ 50.0f, 100.0f });
    componentModule->AddComponent<Health>(e2, Health{ 75.0f, 150.0f });
    componentModule->AddComponent<Health>(e3, Health{ 100.0f, 200.0f });

    SceneDescriptor desc;
    desc.name = "MultiEntityTest";
    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));

    sceneModule->ClearScene();
    EXPECT_TRUE(sceneModule->LoadScene(tempFilePath));

    auto l1 = componentModule->FindEntityByName("Entity1");
    auto l2 = componentModule->FindEntityByName("Entity2");
    auto l3 = componentModule->FindEntityByName("Entity3");

    ASSERT_TRUE(l1.has_value());
    ASSERT_TRUE(l2.has_value());
    ASSERT_TRUE(l3.has_value());

    auto h1 = componentModule->GetComponent<Health>(l1.value());
    auto h2 = componentModule->GetComponent<Health>(l2.value());
    auto h3 = componentModule->GetComponent<Health>(l3.value());

    ASSERT_TRUE(h1.has_value());
    ASSERT_TRUE(h2.has_value());
    ASSERT_TRUE(h3.has_value());

    EXPECT_FLOAT_EQ(h1->get().current, 50.0f);
    EXPECT_FLOAT_EQ(h2->get().current, 75.0f);
    EXPECT_FLOAT_EQ(h3->get().current, 100.0f);
    EXPECT_FLOAT_EQ(h3->get().maximum, 200.0f);
}

TEST_F(SceneModuleTest, SaveAndLoadMultipleComponents)
{
    auto entity = sceneModule->CreateEntity("Player");

    Transform t{};
    t.position = glm::vec3(10.0f, 20.0f, 30.0f);
    componentModule->AddComponent<Transform>(entity, t);
    componentModule->AddComponent<Health>(entity, Health{ 80.0f, 100.0f });

    SceneDescriptor desc;
    desc.name = "MultiComponentTest";
    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));

    sceneModule->ClearScene();
    EXPECT_TRUE(sceneModule->LoadScene(tempFilePath));

    auto loaded = componentModule->FindEntityByName("Player");
    ASSERT_TRUE(loaded.has_value());

    auto transform = componentModule->GetComponent<Transform>(loaded.value());
    auto health = componentModule->GetComponent<Health>(loaded.value());

    ASSERT_TRUE(transform.has_value());
    ASSERT_TRUE(health.has_value());

    EXPECT_FLOAT_EQ(transform->get().position.x, 10.0f);
    EXPECT_FLOAT_EQ(health->get().current, 80.0f);
}

// ============================================================
// Entity ID remapping tests
// ============================================================

TEST_F(SceneModuleTest, EntityIdRemappingOnLoad)
{
    auto entity = sceneModule->CreateEntity("RemapTest");
    componentModule->AddComponent<Health>(entity, Health{ 42.0f, 42.0f });

    SceneDescriptor desc;
    desc.name = "RemapTest";
    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));

    sceneModule->ClearScene();

    // Create some entities directly to offset IDs
    componentModule->CreateEntity();
    componentModule->CreateEntity();
    componentModule->CreateEntity();

    EXPECT_TRUE(sceneModule->LoadScene(tempFilePath));

    auto loaded = componentModule->FindEntityByName("RemapTest");
    ASSERT_TRUE(loaded.has_value());

    // ID should be different from original since we pre-created entities
    auto health = componentModule->GetComponent<Health>(loaded.value());
    ASSERT_TRUE(health.has_value());
    EXPECT_FLOAT_EQ(health->get().current, 42.0f);
}

// ============================================================
// Hierarchy remapping tests
// ============================================================

TEST_F(SceneModuleTest, HierarchyRemappingOnLoad)
{
    auto parent = sceneModule->CreateEntity("Parent");
    auto child1 = sceneModule->CreateEntity("Child1");
    auto child2 = sceneModule->CreateEntity("Child2");

    Hierarchy parentHierarchy{};
    parentHierarchy.children.push_back(child1);
    parentHierarchy.children.push_back(child2);
    componentModule->AddComponent<Hierarchy>(parent, parentHierarchy);

    Hierarchy child1Hierarchy{};
    child1Hierarchy.parent = parent;
    componentModule->AddComponent<Hierarchy>(child1, child1Hierarchy);

    Hierarchy child2Hierarchy{};
    child2Hierarchy.parent = parent;
    componentModule->AddComponent<Hierarchy>(child2, child2Hierarchy);

    SceneDescriptor desc;
    desc.name = "HierarchyTest";
    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));

    sceneModule->ClearScene();

    // Offset IDs
    componentModule->CreateEntity();
    componentModule->CreateEntity();

    EXPECT_TRUE(sceneModule->LoadScene(tempFilePath));

    auto loadedParent = componentModule->FindEntityByName("Parent");
    auto loadedChild1 = componentModule->FindEntityByName("Child1");
    auto loadedChild2 = componentModule->FindEntityByName("Child2");

    ASSERT_TRUE(loadedParent.has_value());
    ASSERT_TRUE(loadedChild1.has_value());
    ASSERT_TRUE(loadedChild2.has_value());

    // Verify parent hierarchy has correct remapped children
    auto ph = componentModule->GetComponent<Hierarchy>(loadedParent.value());
    ASSERT_TRUE(ph.has_value());
    ASSERT_EQ(ph->get().children.size(), 2);
    EXPECT_EQ(ph->get().children[0].id, loadedChild1.value().id);
    EXPECT_EQ(ph->get().children[1].id, loadedChild2.value().id);

    // Verify child1 has correct remapped parent
    auto ch1 = componentModule->GetComponent<Hierarchy>(loadedChild1.value());
    ASSERT_TRUE(ch1.has_value());
    EXPECT_EQ(ch1->get().parent.id, loadedParent.value().id);

    // Verify child2 has correct remapped parent
    auto ch2 = componentModule->GetComponent<Hierarchy>(loadedChild2.value());
    ASSERT_TRUE(ch2.has_value());
    EXPECT_EQ(ch2->get().parent.id, loadedParent.value().id);
}

TEST_F(SceneModuleTest, CustomEntityReferenceRemapping)
{
    auto leader = sceneModule->CreateEntity("Leader");
    auto follower = sceneModule->CreateEntity("Follower");

    componentModule->AddComponent<FollowTarget>(follower, FollowTarget{ leader, 10.0f });

    SceneDescriptor desc;
    desc.name = "FollowTest";
    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));

    sceneModule->ClearScene();

    // Offset IDs
    componentModule->CreateEntity();

    EXPECT_TRUE(sceneModule->LoadScene(tempFilePath));

    auto loadedLeader = componentModule->FindEntityByName("Leader");
    auto loadedFollower = componentModule->FindEntityByName("Follower");

    ASSERT_TRUE(loadedLeader.has_value());
    ASSERT_TRUE(loadedFollower.has_value());

    auto ft = componentModule->GetComponent<FollowTarget>(loadedFollower.value());
    ASSERT_TRUE(ft.has_value());
    EXPECT_EQ(ft->get().target.id, loadedLeader.value().id);
    EXPECT_FLOAT_EQ(ft->get().distance, 10.0f);
}

// ============================================================
// Scene ownership: only owned entities are saved
// ============================================================

TEST_F(SceneModuleTest, NonOwnedEntitiesAreNotSaved)
{
    auto sceneEntity = sceneModule->CreateEntity("SceneEntity");
    componentModule->AddComponent<Health>(sceneEntity, Health{ 50.0f, 100.0f });

    auto runtimeEntity = componentModule->CreateEntity("RuntimeEntity");
    componentModule->AddComponent<Health>(runtimeEntity, Health{ 999.0f, 999.0f });

    SceneDescriptor desc;
    desc.name = "OwnershipTest";
    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));

    sceneModule->ClearScene();

    // Runtime entity should still exist after ClearScene
    EXPECT_TRUE(componentModule->EntityExists(runtimeEntity));

    EXPECT_TRUE(sceneModule->LoadScene(tempFilePath));

    auto loadedScene = componentModule->FindEntityByName("SceneEntity");
    auto loadedRuntime = componentModule->FindEntityByName("RuntimeEntity");

    ASSERT_TRUE(loadedScene.has_value());
    EXPECT_TRUE(loadedRuntime.has_value()); // Still exists, wasn't cleared

    auto health = componentModule->GetComponent<Health>(loadedScene.value());
    ASSERT_TRUE(health.has_value());
    EXPECT_FLOAT_EQ(health->get().current, 50.0f);
}

TEST_F(SceneModuleTest, ClearSceneOnlyDestroysOwnedEntities)
{
    auto sceneEntity = sceneModule->CreateEntity("OwnedEntity");
    auto runtimeEntity = componentModule->CreateEntity("FreeEntity");

    sceneModule->ClearScene();

    EXPECT_FALSE(componentModule->EntityExists(sceneEntity));
    EXPECT_TRUE(componentModule->EntityExists(runtimeEntity));
}

// ============================================================
// Round-trip preservation of unknown components
// ============================================================

TEST_F(SceneModuleTest, UnknownComponentsArePreserved)
{
    // Write a scene file manually with an unknown component
    {
        pugi::xml_document doc;
        auto decl = doc.prepend_child(pugi::node_declaration);
        decl.append_attribute("version").set_value("1.0");
        decl.append_attribute("encoding").set_value("UTF-8");

        auto root = doc.append_child("Scene");
        root.append_attribute("name").set_value("PreservationTest");
        root.append_attribute("version").set_value("1.0");
        root.append_attribute("formatVersion").set_value("1");

        auto entities = root.append_child("Entities");
        entities.append_attribute("count").set_value(1);

        auto entity = entities.append_child("Entity");
        entity.append_attribute("id").set_value(0);
        entity.append_attribute("name").set_value("TestEntity");

        auto health = entity.append_child("Health");
        health.append_attribute("current").set_value(75.0f);
        health.append_attribute("maximum").set_value(100.0f);

        auto unknown = entity.append_child("PhysicsBody");
        unknown.append_attribute("mass").set_value(10.5f);
        unknown.append_attribute("friction").set_value(0.3f);

        doc.save_file(tempFilePath.c_str(), "    ");
    }

    // Load the scene (PhysicsBody has no serializer, should be preserved)
    EXPECT_TRUE(sceneModule->LoadScene(tempFilePath));

    auto loaded = componentModule->FindEntityByName("TestEntity");
    ASSERT_TRUE(loaded.has_value());

    // Health should be deserialized
    auto health = componentModule->GetComponent<Health>(loaded.value());
    ASSERT_TRUE(health.has_value());
    EXPECT_FLOAT_EQ(health->get().current, 75.0f);

    // Re-save the scene
    std::string resavePath = tempFilePath + ".resave.xml";
    SceneDescriptor desc;
    desc.name = "PreservationTest";
    EXPECT_TRUE(sceneModule->SaveScene(resavePath, desc));

    // Verify the unknown component XML was preserved in the re-saved file
    pugi::xml_document resavedDoc;
    ASSERT_TRUE(resavedDoc.load_file(resavePath.c_str()));

    auto entityNode = resavedDoc.child("Scene").child("Entities").child("Entity");
    ASSERT_FALSE(entityNode.empty());

    auto physicsNode = entityNode.child("PhysicsBody");
    EXPECT_FALSE(physicsNode.empty());
    EXPECT_FLOAT_EQ(physicsNode.attribute("mass").as_float(), 10.5f);
    EXPECT_FLOAT_EQ(physicsNode.attribute("friction").as_float(), 0.3f);

    std::filesystem::remove(resavePath);
}

// ============================================================
// Error handling
// ============================================================

TEST_F(SceneModuleTest, LoadNonExistentFileFails)
{
    EXPECT_FALSE(sceneModule->LoadScene("nonexistent_file.xml"));
    EXPECT_FALSE(sceneModule->HasActiveScene());
}

TEST_F(SceneModuleTest, LoadInvalidXmlFails)
{
    std::ofstream out(tempFilePath);
    out << "this is not valid xml";
    out.close();

    EXPECT_FALSE(sceneModule->LoadScene(tempFilePath));
    EXPECT_FALSE(sceneModule->HasActiveScene());
}

TEST_F(SceneModuleTest, LoadXmlWithoutSceneRootFails)
{
    std::ofstream out(tempFilePath);
    out << "<?xml version=\"1.0\"?><NotAScene/>";
    out.close();

    EXPECT_FALSE(sceneModule->LoadScene(tempFilePath));
    EXPECT_FALSE(sceneModule->HasActiveScene());
}

// ============================================================
// Scene descriptor tests
// ============================================================

TEST_F(SceneModuleTest, SceneDescriptorRoundTrip)
{
    sceneModule->CreateEntity("Dummy");

    SceneDescriptor desc;
    desc.name = "My Scene";
    desc.version = "2.5";
    desc.author = "TestAuthor";
    desc.description = "A description of the scene";

    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));

    sceneModule->ClearScene();
    EXPECT_TRUE(sceneModule->LoadScene(tempFilePath));

    const auto& loaded = sceneModule->GetCurrentSceneDescriptor();
    EXPECT_EQ(loaded.name, "My Scene");
    EXPECT_EQ(loaded.version, "2.5");
    EXPECT_EQ(loaded.author, "TestAuthor");
    EXPECT_EQ(loaded.description, "A description of the scene");
}

// ============================================================
// XML file format verification
// ============================================================

TEST_F(SceneModuleTest, SavedXmlHasCorrectStructure)
{
    auto entity = sceneModule->CreateEntity("TestObj");

    Transform t{};
    t.position = glm::vec3(5.0f, 10.0f, 15.0f);
    componentModule->AddComponent<Transform>(entity, t);

    SceneDescriptor desc;
    desc.name = "StructureTest";
    desc.version = "1.0";
    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));

    pugi::xml_document doc;
    ASSERT_TRUE(doc.load_file(tempFilePath.c_str()));

    auto root = doc.child("Scene");
    EXPECT_FALSE(root.empty());
    EXPECT_STREQ(root.attribute("name").as_string(), "StructureTest");
    EXPECT_STREQ(root.attribute("formatVersion").as_string(), "1");

    auto entities = root.child("Entities");
    EXPECT_FALSE(entities.empty());
    EXPECT_EQ(entities.attribute("count").as_int(), 1);

    auto entityNode = entities.child("Entity");
    EXPECT_FALSE(entityNode.empty());
    EXPECT_STREQ(entityNode.attribute("name").as_string(), "TestObj");

    auto transformNode = entityNode.child("Transform");
    EXPECT_FALSE(transformNode.empty());

    auto posNode = transformNode.child("Position");
    EXPECT_FALSE(posNode.empty());
    EXPECT_FLOAT_EQ(posNode.attribute("x").as_float(), 5.0f);
    EXPECT_FLOAT_EQ(posNode.attribute("y").as_float(), 10.0f);
    EXPECT_FLOAT_EQ(posNode.attribute("z").as_float(), 15.0f);
}

// ============================================================
// ComponentSerializerRegistry tests
// ============================================================

TEST_F(SceneModuleTest, UnregisteredComponentIsNotSerialized)
{
    struct Unregistered { int value = 42; };

    auto entity = sceneModule->CreateEntity("TestObj");
    componentModule->AddComponent<Unregistered>(entity, Unregistered{ 42 });

    SceneDescriptor desc;
    desc.name = "UnregisteredTest";
    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));

    // Verify no <Unregistered> element exists in the XML
    pugi::xml_document doc;
    ASSERT_TRUE(doc.load_file(tempFilePath.c_str()));

    auto entityNode = doc.child("Scene").child("Entities").child("Entity");
    EXPECT_FALSE(entityNode.empty());

    // Only the entity node itself should exist, no component children
    // (since Unregistered has no serializer)
    int childCount = 0;
    for (auto child = entityNode.first_child(); child; child = child.next_sibling())
        childCount++;

    EXPECT_EQ(childCount, 0);
}

// ============================================================
// Double save/load cycle
// ============================================================

TEST_F(SceneModuleTest, DoubleSaveLoadCycle)
{
    auto entity = sceneModule->CreateEntity("Persistent");
    componentModule->AddComponent<Health>(entity, Health{ 33.0f, 66.0f });

    SceneDescriptor desc;
    desc.name = "CycleTest";

    // First save/load
    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));
    sceneModule->ClearScene();
    EXPECT_TRUE(sceneModule->LoadScene(tempFilePath));

    // Second save/load
    EXPECT_TRUE(sceneModule->SaveScene(tempFilePath, desc));
    sceneModule->ClearScene();
    EXPECT_TRUE(sceneModule->LoadScene(tempFilePath));

    auto loaded = componentModule->FindEntityByName("Persistent");
    ASSERT_TRUE(loaded.has_value());

    auto health = componentModule->GetComponent<Health>(loaded.value());
    ASSERT_TRUE(health.has_value());
    EXPECT_FLOAT_EQ(health->get().current, 33.0f);
    EXPECT_FLOAT_EQ(health->get().maximum, 66.0f);
}
